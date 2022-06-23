/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include "Analyzer.h"

namespace ObjectiveNinja {

/**
 * Common analysis and info interface.
 */
class AnalysisProvider {
public:
    /**
     * Run the default suite of analyzers on an abstract file and get the
     * resulting AnalysisInfo.
     */
    static SharedAnalysisInfo infoForFile(SharedAbstractFile);
};

}
