/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include <ObjectiveNinjaCore/Analyzers/ClassAnalyzer.h>

using namespace ObjectiveNinja;

ClassAnalyzer::ClassAnalyzer(SharedAnalysisInfo info,
    SharedAbstractFile file)
    : Analyzer(std::move(info), std::move(file))
{
}

void ClassAnalyzer::run()
{
    const auto sectionStart = m_file->sectionStart("__objc_classlist");
    const auto sectionEnd = m_file->sectionEnd("__objc_classlist");
    if (sectionStart == 0 || sectionEnd == 0)
        return;

    for (auto address = sectionStart; address < sectionEnd; address += 8) {
        ClassInfo ci;
        ci.listPointer = address;
        ci.address = uiro(m_file->readLong(address));
        ci.dataAddress = uiro(m_file->readLong(ci.address + 0x20));
        ci.nameAddress = uiro(m_file->readLong(ci.dataAddress + 0x18));
        ci.methodListAddress = uiro(m_file->readLong(ci.dataAddress + 0x20));

        m_info->classes.emplace_back(ci);
    }
}
