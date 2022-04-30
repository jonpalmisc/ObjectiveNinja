/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include "Workflow.h"

#include "../CustomTypes.h"
#include "../GlobalState.h"
#include "../InfoHandler.h"

#include <ObjectiveNinjaCore/AnalysisProvider.h>
#include <ObjectiveNinjaCore/Support/BinaryViewFile.h>

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
    uint64_t rawSelector = ssa->GetSSARegisterValue(selectorRegister).value;

    // Check the analysis info for a selector reference corresponding to the
    // current selector. It is possible no such selector reference exists, for
    // example, if the selector is for a method defined outside the current
    // binary. If this is the case, there are no meaningful changes that can be
    // made to the IL, and the operation should be aborted.
    const auto info = GlobalState::analysisInfo(bv);
    if (!info->selectorRefsByKey.count(rawSelector))
        return;
    const auto selectorRef = info->selectorRefsByKey[rawSelector];

    // Attempt to look up the implementation for the given selector, first by
    // using the raw selector, then by the address of the selector reference. If
    // the lookup fails in both cases, abort.
    uint64_t implAddress = info->methodImpls[selectorRef->rawSelector];
    if (!implAddress)
        implAddress = info->methodImpls[selectorRef->address];
    if (!implAddress)
        return;

    const auto llilIndex = ssa->GetNonSSAInstructionIndex(insnIndex);
    auto llilInsn = llil->GetInstruction(llilIndex);

    // Change the destination expression of the LLIL_CALL operation to point to
    // the method implementation. This turns the "indirect call" piped through
    // `objc_msgSend` and makes it a normal C-style function call.
    auto callDestExpr = llilInsn.GetDestExpr<LLIL_CALL>();
    callDestExpr.Replace(llil->ConstPointer(callDestExpr.size, implAddress, callDestExpr));
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

    const auto log = BinaryNinja::LogRegistry::GetLogger("ObjectiveNinja");

    // Ignore the view if it has an unsupported architecture.
    auto archName = arch->GetName();
    if (archName != "aarch64" && archName != "x86_64") {
        log->LogError("Architecture '%s' is not supported", archName.c_str());
        GlobalState::addIgnoredView(bv);
        return;
    }

    // The workflow relies on some data acquired through analysis of Objective-C
    // structures present in the binary. The structure analysis must run
    // exactly once per binary. Until the Workflows API supports a "run once"
    // idiom, this is accomplished through a mutex and a check for present
    // analysis information.
    {
        std::scoped_lock<std::mutex> lock(g_initialAnalysisMutex);

        if (!GlobalState::hasAnalysisInfo(bv)) {
            SharedAnalysisInfo info;
            CustomTypes::defineAll(bv);

            try {
                auto file = std::make_shared<ObjectiveNinja::BinaryViewFile>(bv);
                info = ObjectiveNinja::AnalysisProvider::infoForFile(file);

                InfoHandler::applyInfoToView(info, bv);
            } catch (...) {
                log->LogError("Analysis failed. Please report this bug!");
            }

            GlobalState::setFlag(bv, Flag::DidRunStructureAnalysis);
            GlobalState::storeAnalysisInfo(bv, info);
        }
    }

    // Try to find the `objc_msgSend` functions(s), abort activity if missing.
    //
    // TODO: These results should be cached somehow as it can't be efficient to
    // repeatedly search for all the usable function addresses.
    const auto msgSendFunctions = findMsgSendFunctions(bv);
    if (msgSendFunctions.empty()) {
        log->LogError("Cannot perform Objective-C IL cleanup; no objc_msgSend candidates found");
        GlobalState::addIgnoredView(bv);
        return;
    }

    const auto llil = ac->GetLowLevelILFunction();
    if (!llil) {
        log->LogError("(Workflow) Failed to get LLIL for 0x%llx", func->GetStart());
        return;
    }
    const auto ssa = llil->GetSSAForm();
    if (!ssa) {
        log->LogError("(Workflow) Failed to get LLIL SSA form for 0x%llx", func->GetStart());
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
