/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include "Commands.h"

#include "CustomTypes.h"
#include "GlobalState.h"
#include "InfoHandler.h"
#include "Performance.h"

#include "Core/AnalysisProvider.h"
#include "Core/BinaryViewFile.h"

void Commands::defineTypes(BinaryViewRef bv)
{
    CustomTypes::defineAll(std::move(bv));
}

void Commands::analyzeStructures(BinaryViewRef bv)
{
    if (GlobalState::hasFlag(bv, Flag::DidRunWorkflow)
        || GlobalState::hasFlag(bv, Flag::DidRunStructureAnalysis)) {
        auto result = BinaryNinja::ShowMessageBox("Error",
            "Structure analysis has already been performed fon this binary. "
            "Repeated analysis may cause unexpected behavior.* Continue?\n\n"
            "*If you undid analysis, this message can be safely ignored.",
            BNMessageBoxButtonSet::YesNoButtonSet,
            BNMessageBoxIcon::QuestionIcon);

        if (result != BNMessageBoxButtonResult::YesButton)
            return;
    }

    SharedAnalysisInfo info;
    CustomTypes::defineAll(bv);

    try {
        auto file = std::make_shared<ObjectiveNinja::BinaryViewFile>(bv);

        auto start = Performance::now();
        info = ObjectiveNinja::AnalysisProvider::infoForFile(file);
        auto elapsed = Performance::elapsed<std::chrono::milliseconds>(start);

        const auto log = BinaryNinja::LogRegistry::GetLogger("ObjectiveNinja");
        log->LogInfo("Structures analyzed in %lu ms", elapsed.count());

        InfoHandler::applyInfoToView(info, bv);
    } catch (...) {
        const auto log = BinaryNinja::LogRegistry::GetLogger("ObjectiveNinja");
        log->LogError("Structure analysis failed; binary may be malformed.");
        log->LogError("Objective-C analysis will not be applied due to previous errors.");
    }

    GlobalState::setFlag(bv, Flag::DidRunWorkflow);
}

void Commands::registerCommands()
{
#ifdef DEV_MODE
    BinaryNinja::PluginCommand::Register("Objective Ninja \\ Define Types",
        "", Commands::defineTypes);
#endif
    BinaryNinja::PluginCommand::Register("Objective Ninja \\ Analyze Structures",
        "", Commands::analyzeStructures);
}
