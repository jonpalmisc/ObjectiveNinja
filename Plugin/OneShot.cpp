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

#include "OneShot.hpp"

#include "CustomTypes.hpp"
#include "GlobalState.hpp"
#include "InfoHandler.h"
#include "StructureAnalyzer.hpp"
#include "Support/BinaryViewFile.h"

#include <ObjectiveNinjaCore/Analyzers/CFStringAnalyzer.h>

void OneShot::defineTypes(BinaryNinja::BinaryView* bv)
{
    CustomTypes::defineAll(bv);
}

void OneShot::analyzeStructures(BinaryNinja::BinaryView* bv)
{
    if (GlobalState::hasFlag(bv, Flag::DidRunWorkflow)
        || GlobalState::hasFlag(bv, Flag::DidRunStructureAnalysis)) {
        BinaryNinja::ShowMessageBox("Error",
            "Structure analysis has already been performed on this binary.");
        return;
    }

    CustomTypes::defineAll(bv);
    StructureAnalyzer::run(bv);

    GlobalState::setFlag(bv, Flag::DidRunWorkflow);
}

void OneShot::registerCommands()
{
#ifdef DEV_MODE
    auto runNewAnalysis = [](BinaryNinja::BinaryView* bv) {
        CustomTypes::defineAll(bv);

        auto bvFile = std::make_shared<BinaryViewFile>(bv);
        auto info = std::make_shared<ObjectiveNinja::AnalysisInfo>();
        auto analyzers = {
            std::make_unique<ObjectiveNinja::CFStringAnalyzer>(info, bvFile),
        };

        for (const auto& analyzer : analyzers)
            analyzer->run();

        InfoHandler::applyInfoToView(info, bv);
    };

    BinaryNinja::PluginCommand::Register("Objective Ninja \\ Run Analysis 2.0",
        "", runNewAnalysis);
    BinaryNinja::PluginCommand::Register("Objective Ninja \\ Define Types",
        "", OneShot::defineTypes);
#endif
    BinaryNinja::PluginCommand::Register("Objective Ninja \\ Analyze Structures",
        "", OneShot::analyzeStructures);
}
