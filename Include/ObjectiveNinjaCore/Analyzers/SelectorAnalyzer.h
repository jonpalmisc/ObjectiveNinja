/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include <ObjectiveNinjaCore/Analyzer.h>

namespace ObjectiveNinja {

/**
 * Analyzer for parsing Objective-C selectors and selector references.
 */
class SelectorAnalyzer : public Analyzer {
public:
    SelectorAnalyzer(SharedAnalysisInfo, SharedAbstractFile);

    void run() override;
};

}
