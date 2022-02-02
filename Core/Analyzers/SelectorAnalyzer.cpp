/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include <ObjectiveNinjaCore/Analyzers/SelectorAnalyzer.h>

using namespace ObjectiveNinja;

SelectorAnalyzer::SelectorAnalyzer(SharedAnalysisInfo info,
    SharedAbstractFile file)
    : Analyzer(std::move(info), std::move(file))
{
}

void SelectorAnalyzer::run()
{
    const auto sectionStart = m_file->sectionStart("__objc_selrefs");
    const auto sectionEnd = m_file->sectionEnd("__objc_selrefs");
    if (sectionStart == 0 || sectionEnd == 0)
        return;

    for (auto address = sectionStart; address < sectionEnd; address += 0x8) {
        auto ssri = std::make_shared<SelectorRefInfo>();
        ssri->address = address;
        ssri->rawSelector = m_file->readLong(address);
        ssri->nameAddress = arp(ssri->rawSelector);
        ssri->name = m_file->readString(ssri->nameAddress);

        m_info->selectorRefs[ssri->rawSelector] = ssri;
        m_info->selectorRefs[ssri->address] = ssri;
    }
}
