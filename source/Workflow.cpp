/*
 * Copyright (c) 2022 Jon Palmisciano
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "Workflow.hpp"

#include "AnalysisRecords.hpp"
#include "CustomTypes.hpp"
#include "GlobalState.hpp"
#include "StructureAnalyzer.hpp"

#include <binaryninjaapi.h>
#include <lowlevelilinstruction.h>

#include <queue>

static std::mutex g_initialAnalysisMutex;

using SectionRef = BinaryNinja::Ref<BinaryNinja::Section>;
using SymbolRef = BinaryNinja::Ref<BinaryNinja::Symbol>;

std::set<uint64_t> Workflow::findMsgSendFunctions(BinaryViewRef bv)
{
    std::set<uint64_t> results;

    const auto authStubsSection = bv->GetSectionByName("__auth_stubs");
    const auto stubsSection = bv->GetSectionByName("__stubs");
    const auto authGotSection = bv->GetSectionByName("__auth_got");
    const auto gotSection = bv->GetSectionByName("__got");
    const auto laSymbolPtrSection = bv->GetSectionByName("__la_symbol_ptr");

    // Shorthand to check if a symbol lies in a given section.
    auto sectionContains = [](SectionRef section, SymbolRef symbol) {
        const auto start = section->GetStart();
        const auto length = section->GetLength();
        const auto address = symbol->GetAddress();

        return (uint64_t)(address - start) <= length;
    };

    // There can be multiple `_objc_msgSend` symbols in the same binary; there
    // may even be lots. Some of them are valid, others aren't. In order of
    // preference, `_objc_msgSend` symbols in the following sections are
    // preferred:
    //
    //   1. __auth_stubs
    //   2. __stubs
    //   3. __auth_got
    //   4. __got
    //   ?. __la_symbol_ptr
    //
    // There is often an `_objc_msgSend` symbol that is a stub function, found
    // in the `__stubs` section, which will come with an imported symbol of the
    // same name in the `__got` section. Not all `__objc_msgSend` calls will be
    // routed through the stub function, making it important to make note of
    // both symbols' addresses. Furthermore, on ARM64, the `__auth{stubs,got}`
    // sections are preferred over their unauthenticated counterparts.
    const auto candidates = bv->GetSymbolsByName("_objc_msgSend");
    for (const auto& c : candidates) {
        if ((authStubsSection && sectionContains(authStubsSection, c))
            || (stubsSection && sectionContains(stubsSection, c))
            || (authGotSection && sectionContains(authGotSection, c))
            || (gotSection && sectionContains(gotSection, c))
            || (laSymbolPtrSection && sectionContains(laSymbolPtrSection, c)))
            results.insert(c->GetAddress());
    }

    return results;
}

void Workflow::rewriteMethodCall(LLILFunctionRef ssa, size_t insnIndex)
{
    const auto bv = ssa->GetFunction()->GetView();
    const auto llil = ssa->GetNonSSAForm();
    const auto insn = ssa->GetInstruction(insnIndex);
    const auto params = insn.GetParameterExprs<LLIL_CALL_SSA>();

    // The second parameter passed to the objc_msgSend call is the address of
    // either the selector reference or the method's name, which in both cases
    // is dereferenced to retrieve a selector.
    const auto selectorRegister = params[1].GetSourceSSARegister<LLIL_REG_SSA>();
    uint64_t selector = ssa->GetSSARegisterValue(selectorRegister).value;
    selector &= 0xFFFFFFFF;
    selector += bv->GetStart();

    // Check the analysis records for an implementation address corresponding to
    // the current selector. It is possible that no implementation address
    // exists, for example, when the selector is for a method defined outside
    // the current binary. If this is the case, there are no meaningful changes
    // that can be made to the IL, and the operation should be aborted.
    const auto impAddress = GlobalState::analysisRecords(bv)->impMap[selector];
    if (!impAddress)
        return;

    const auto llilIndex = ssa->GetNonSSAInstructionIndex(insnIndex);
    auto llilInsn = llil->GetInstruction(llilIndex);

    // Change the destination expression of the LLIL_CALL operation to point to
    // the method implementation. This turns the "indirect call" piped through
    // `objc_msgSend` and makes it a normal C-style function call.
    auto callDestExpr = llilInsn.GetDestExpr<LLIL_CALL>();
    callDestExpr.Replace(llil->ConstPointer(callDestExpr.size, impAddress, callDestExpr));
    llilInsn.Replace(llil->Call(callDestExpr.exprIndex, llilInsn));

    llil->GenerateSSAForm();
}

void Workflow::inlineMethodCalls(AnalysisContextRef ac)
{
    const auto func = ac->GetFunction();
    const auto arch = func->GetArchitecture();
    const auto bv = func->GetView();

    if (GlobalState::viewIsIgnored(bv))
        return;

    // Ignore the view if it has an unsupported architecture.
    auto archName = arch->GetName();
    if (archName != "aarch64" && archName != "x86_64") {
        BinaryNinja::LogError("Architecture '%s' not supported by Objective Ninja",
            archName.c_str());
        GlobalState::addIgnoredView(bv);
        return;
    }

    // The workflow relies on some data acquired through analysis of Objective-C
    // structures present in the binary. The structure analysis must run exactly
    // once per binary. Until the Workflows API supports a "run once" idiom,
    // this is accomplished through a mutex and a map of analysis records, which
    // also serves as the list of binaries that have had structure analysis.
    {
        std::scoped_lock<std::mutex> lock(g_initialAnalysisMutex);

        if (!GlobalState::hasAnalysisRecords(bv)) {
            CustomTypes::defineAll(bv);
            auto analysisRecords = StructureAnalyzer::run(bv);

            GlobalState::setFlag(bv, Flag::DidRunStructureAnalysis);
            GlobalState::storeAnalysisRecords(bv, analysisRecords);
        }
    }

    // Try to find the `objc_msgSend` functions(s), abort activity if missing.
    //
    // TODO: These results should be cached somehow as it can't be efficient to
    // repeatedly search for all the usable function addresses.
    const auto msgSendFunctions = findMsgSendFunctions(bv);
    if (msgSendFunctions.empty()) {
        BinaryNinja::LogError("Cannot perform Objective-C IL cleanup; no objc_msgSend candidates found");
        GlobalState::addIgnoredView(bv);
        return;
    }

    const auto llil = ac->GetLowLevelILFunction();
    if (!llil) {
        BinaryNinja::LogError("Bad result from `ac->GetLowLevelILFunction()`");
        return;
    }
    const auto ssa = llil->GetSSAForm();
    if (!ssa) {
        BinaryNinja::LogError("Bad result from `llil->GetSSAForm()`");
        return;
    }

    const auto rewriteIfEligible = [msgSendFunctions, ssa](size_t insnIndex) {
        auto insn = ssa->GetInstruction(insnIndex);

        if (insn.operation != LLIL_CALL_SSA)
            return;

        // Filter out calls that aren't to `objc_msgSend`.
        auto callExpr = insn.GetDestExpr<LLIL_CALL_SSA>();
        if (!msgSendFunctions.count(callExpr.GetValue().value))
            return;

        // By convention, the selector is the second argument to `objc_msgSend`,
        // therefore two parameters are required for a proper rewrite; abort if
        // this condition is not met.
        auto params = insn.GetParameterExprs<LLIL_CALL_SSA>();
        if (params.size() < 2
            || params[0].operation != LLIL_REG_SSA
            || params[1].operation != LLIL_REG_SSA)
            return;

        rewriteMethodCall(ssa, insnIndex);
    };

    for (const auto& block : ssa->GetBasicBlocks())
        for (size_t i = block->GetStart(), end = block->GetEnd(); i < end; ++i)
            rewriteIfEligible(i);
}

static constexpr auto WorkflowInfo = R"({
  "title": "Objective Ninja",
  "description": "Enhanced analysis for Objective-C code.",
  "capabilities": []
})";

void Workflow::registerActivities()
{
    const auto wf = BinaryNinja::Workflow::Instance()->Clone("Objective Ninja");
    wf->RegisterActivity(new BinaryNinja::Activity(
        ActivityID::InlineMethodCalls, &Workflow::inlineMethodCalls));
    wf->Insert("core.function.translateTailCalls", ActivityID::InlineMethodCalls);

    BinaryNinja::Workflow::RegisterWorkflow(wf, WorkflowInfo);
}
