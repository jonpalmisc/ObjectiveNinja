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

#pragma once

#include "AnalysisRecords.hpp"
#include "Workflow.hpp"

/// Namepace to hold metadata flag key constants.
namespace Flag {

constexpr auto DidRunWorkflow = "objectiveNinja.didRunWorkflow";
constexpr auto DidRunStructureAnalysis = "objectiveNinja.didRunStructureAnalysis";

}

/// Global state/storage interface.
class GlobalState {
    /// Get the ID for a view.
    static std::uintptr_t id(BinaryViewRef);

public:
    /// Get the analysis records for a view.
    static AnalysisRecords* analysisRecords(BinaryViewRef);

    /// Store the analysis records for a view.
    static void storeAnalysisRecords(BinaryViewRef, AnalysisRecords);

    /// Check if analysis records exist for a view.
    static bool hasAnalysisRecords(BinaryViewRef);

    /// Add a view to the list of ignored views.
    static void addIgnoredView(BinaryViewRef);

    /// Check if a view is ignored.
    static bool viewIsIgnored(BinaryViewRef);

    /// Check if the a metadata flag is present for a view.
    static bool hasFlag(BinaryViewRef, const std::string&);

    /// Set a metadata flag for a view.
    static void setFlag(BinaryViewRef, const std::string&);
};
