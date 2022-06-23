/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include "AnalysisInfo.h"
#include "TypeParser.h"

namespace ObjectiveNinja {

constexpr auto FlagsMask = 0xFFFF0000;

std::vector<std::string> MethodInfo::selectorTokens() const
{
    std::stringstream r(selector);

    std::string token;
    std::vector<std::string> result;
    while (std::getline(r, token, ':'))
        result.push_back(token);

    return result;
}

std::vector<std::string> MethodInfo::decodedTypeTokens() const
{
    return TypeParser::parseEncodedType(type);
}

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
    nlohmann::json json;
    json["classes"] = classes;
    json["cfStrings"] = cfStrings;

    return json.dump(2);
}

}
