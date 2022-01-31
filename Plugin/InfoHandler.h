/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include <ObjectiveNinjaCore/AnalysisInfo.h>

#include <binaryninjaapi.h>

using BinaryViewRef = BinaryNinja::Ref<BinaryNinja::BinaryView>;
using SharedAnalysisInfo = std::shared_ptr<ObjectiveNinja::AnalysisInfo>;

/**
 * Utility class for applying collected AnalysisInfo to a database.
 *
 * InfoHandler is meant to be used after all analyzers intended to run on a
 * database have finished. The resulting AnalysisInfo will then be used to
 * create data variables, symbols, etc. in the database.
 */
class InfoHandler {
public:
    /**
     * Apply AnalysisInfo to a BinaryView.
     */
    static void applyInfoToView(SharedAnalysisInfo, BinaryViewRef);
};
