/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ObjectiveNinja {

/**
 * A description of a CFString instance.
 */
struct CFStringInfo {
    uint64_t address {};
    uint64_t dataAddress {};
    size_t size {};
};

/**
 * Analysis info storage.
 *
 * AnalysisInfo is intended to be a common structure for persisting information
 * during and after analysis. All significant info obtained or produced through
 * analysis should be stored here, ideally in the form of other *Info structs.
 */
struct AnalysisInfo {
    std::vector<CFStringInfo> cfStrings;
};

}
