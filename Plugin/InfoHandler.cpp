/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include "InfoHandler.h"

#include <regex>

using namespace BinaryNinja;

std::string InfoHandler::sanitizeText(const std::string& text)
{
    std::string result;
    std::string input = text.substr(0, 24);

    std::regex re("[a-zA-Z]+");
    std::smatch sm;
    while (std::regex_search(input, sm, re)) {
        std::string part = sm[0];
        part[0] = static_cast<char>(std::toupper(part[0]));

        result += part;
        input = sm.suffix();
    }

    return result;
}

void InfoHandler::applyInfoToView(SharedAnalysisInfo info, BinaryViewRef bv)
{
    BinaryReader reader(bv);

    auto cfStringType = Type::NamedType(bv, std::string("CFString"));
    for (auto csi : info->cfStrings) {
        bv->DefineUserDataVariable(csi.address, cfStringType);

        auto stringType = Type::ArrayType(Type::IntegerType(1, true), csi.size + 1);
        bv->DefineUserDataVariable(csi.dataAddress, stringType);

        reader.Seek(csi.dataAddress);
        auto text = reader.ReadString(csi.size + 1);
        auto sanitizedText = sanitizeText(text);

        auto s = new Symbol(DataSymbol, "cfs_" + sanitizedText, csi.address);
        bv->DefineUserSymbol(s);
    }
}
