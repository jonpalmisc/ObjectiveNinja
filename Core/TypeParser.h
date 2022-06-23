/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include <string>
#include <vector>

namespace ObjectiveNinja {

/**
 * Parser for Objective-C type strings.
 */
class TypeParser {
public:
    /**
     * Parse an encoded type string.
     */
    static std::vector<std::string> parseEncodedType(const std::string&);
};

}
