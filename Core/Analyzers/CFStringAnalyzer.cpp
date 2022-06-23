/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include "CFStringAnalyzer.h"

using namespace ObjectiveNinja;

CFStringAnalyzer::CFStringAnalyzer(SharedAnalysisInfo info,
    SharedAbstractFile file)
    : Analyzer(std::move(info), std::move(file))
{
}

void CFStringAnalyzer::run()
{
    const auto sectionStart = m_file->sectionStart("__cfstring");
    const auto sectionEnd = m_file->sectionEnd("__cfstring");
    if (sectionStart == 0 || sectionEnd == 0)
        return;

    for (auto address = sectionStart; address < sectionEnd; address += 0x20) {
        CFStringInfo cfString;
        cfString.address = address;
        cfString.dataAddress = arp(m_file->readLong(address + 0x10));
        cfString.size = m_file->readLong(address + 0x18);

        m_info->cfStrings.emplace_back(cfString);
    }
}
