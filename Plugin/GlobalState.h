/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include "BinaryNinja.h"

#include <ObjectiveNinjaCore/AnalysisInfo.h>

using SharedAnalysisInfo = std::shared_ptr<ObjectiveNinja::AnalysisInfo>;

/**
 * Namepace to hold metadata flag key constants.
 */
namespace Flag {

constexpr auto DidRunWorkflow = "objectiveNinja.didRunWorkflow";
constexpr auto DidRunStructureAnalysis = "objectiveNinja.didRunStructureAnalysis";

}

/**
 * Global state/storage interface.
 */
class GlobalState {
    /**
     * Get the ID for a view.
     */
    static std::uintptr_t id(BinaryViewRef);

public:
    /**
     * Get the analysis info for a view.
     */
    static SharedAnalysisInfo analysisInfo(BinaryViewRef);

    /**
     * Store analysis info for a view.
     */
    static void storeAnalysisInfo(BinaryViewRef, SharedAnalysisInfo);

    /**
     * Check if analysis info exists for a view.
     */
    static bool hasAnalysisInfo(BinaryViewRef);

    /**
     * Add a view to the list of ignored views.
     */
    static void addIgnoredView(BinaryViewRef);

    /**
     * Check if a view is ignored.
     */
    static bool viewIsIgnored(BinaryViewRef);

    /**
     * Check if the a metadata flag is present for a view.
     */
    static bool hasFlag(BinaryViewRef, const std::string&);

    /**
     * Set a metadata flag for a view.
     */
    static void setFlag(BinaryViewRef, const std::string&);
};
