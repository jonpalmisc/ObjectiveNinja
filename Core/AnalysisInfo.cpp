/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include <ObjectiveNinjaCore/AnalysisInfo.h>

namespace ObjectiveNinja {

constexpr auto FlagsMask = 0xFFFF0000;

bool MethodListInfo::hasRelativeOffsets() const
{
    return (flags & FlagsMask) & 0x80000000;
}

bool MethodListInfo::hasDirectSelectors() const
{
    return (flags & FlagsMask) & 0x40000000;
}

}
