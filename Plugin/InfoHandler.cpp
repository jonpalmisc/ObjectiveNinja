/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include "InfoHandler.h"

void InfoHandler::applyInfoToView(SharedAnalysisInfo info, BinaryViewRef bv)
{
    auto cfsType = BinaryNinja::Type::NamedType(bv, std::string("CFString"));
    for (auto csi : info->cfStrings) {
        bv->DefineUserDataVariable(csi.address, cfsType);
    }
}
