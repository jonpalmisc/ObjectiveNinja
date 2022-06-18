/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include <ObjectiveNinjaCore/TypeParser.h>

#include <map>

namespace ObjectiveNinja {

static const std::map<char, std::string> TypeEncodingMap = {
    { 'v', "void" },
    { 'c', "char" },
    { 's', "short" },
    { 'i', "int" },
    { 'l', "long" },
    { 'C', "unsigned char" },
    { 'S', "unsigned short" },
    { 'I', "unsigned int" },
    { 'L', "unsigned long" },
    { 'f', "float" },
    { 'A', "uint8_t" },
    { 'b', "BOOL" },
    { 'B', "BOOL" },

    { 'q', "NSInteger" },
    { 'Q', "NSUInteger" },
    { 'd', "CGFloat" },
    { '*', "char *" },

    { '@', "id" },
    { ':', "SEL" },
    { '#', "objc_class_t" },

    { '?', "void*" },
    { 'T', "void*" },
};

std::vector<std::string> TypeParser::parseEncodedType(const std::string& encodedType)
{
    std::vector<std::string> result;

    for (size_t i = 0; i < encodedType.size(); ++i) {
        char c = encodedType[i];

        // Argument frame size and offset specifiers aren't relevant here; they
        // should just be skipped.
        if (std::isdigit(c))
            continue;

        if (TypeEncodingMap.count(c)) {
            result.emplace_back(TypeEncodingMap.at(c));
            continue;
        }

        // (Partially) handle quoted type names.
        if (c == '"') {
            while (encodedType[i] != '"')
                i++;

            // TODO: Emit real type names.
            result.emplace_back("void*");
            continue;
        }

        // (Partially) handle struct types.
        if (c == '{') {
            auto depth = 1;

            while (depth != 0) {
                char d = encodedType[++i];

                if (d == '{')
                    ++depth;
                else if (d == '}')
                    --depth;
            }

            // TODO: Emit real struct types.
            result.emplace_back("void*");
            continue;
        }

        break;
    }

    return result;
}

}
