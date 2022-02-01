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

std::string AnalysisInfo::dump() const
{
    std::vector<MethodListInfo> methodListValues;
    for (const auto& p : methodLists)
        methodListValues.emplace_back(p.second);

    std::vector<SelectorRefInfo> filteredSelectorRefs;
    for (const auto& [k, v] : selectorRefs) {
        if (k < 0x200000000)
            continue;

        filteredSelectorRefs.emplace_back(*v);
    }

    nlohmann::json j;
    j["classes"] = classes;
    j["methodLists"] = methodListValues;
    j["selectorRefs"] = filteredSelectorRefs;
    j["cfStrings"] = cfStrings;

    return j.dump(2);
}

}
