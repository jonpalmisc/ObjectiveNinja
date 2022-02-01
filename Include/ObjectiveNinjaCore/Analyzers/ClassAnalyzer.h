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
 * Analyzer for extracting Objective-C class information.
 *
 * ClassAnalyzer starts by processing the `__objc_classlist` section, which
 * defines a list of classes. Each class listed (and its related data) will
 * then be analyzed.
 */
class ClassAnalyzer : public Analyzer {
    /**
     * Analyze a method list.
     *
     * This function is extracted from the `run()` method since analyzing method
     * lists can be tricky and has a handful of edge cases. It's easier to see
     * what's going on when it's separated from the rest of the analysis code.
     */
    MethodListInfo analyzeMethodList(uint64_t);

public:
    ClassAnalyzer(SharedAnalysisInfo, SharedAbstractFile);

    void run() override;
};

}
