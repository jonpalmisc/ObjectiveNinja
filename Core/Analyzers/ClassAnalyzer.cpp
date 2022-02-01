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
        MethodListInfo mli;

        ci.listPointer = address;
        ci.address = uiro(m_file->readLong(address));
        ci.dataAddress = uiro(m_file->readLong(ci.address + 0x20));
        ci.nameAddress = uiro(m_file->readLong(ci.dataAddress + 0x18));
        ci.name = m_file->readString(ci.nameAddress);
        ci.methodListAddress = uiro(m_file->readLong(ci.dataAddress + 0x20));

        mli.address = ci.methodListAddress;

        auto methodCount = m_file->readInt(mli.address + 0x4);
        for (auto i = 0; i < methodCount; ++i) {
            MethodInfo mi;
            mi.address = mli.address + 8 + (i * 12);

            m_file->seek(mi.address);
            mi.nameAddress = mi.address + static_cast<int32_t>(m_file->readInt());
            mi.typeAddress = mi.address + 4 + static_cast<int32_t>(m_file->readInt());
            mi.implAddress = mi.address + 8 + static_cast<int32_t>(m_file->readInt());

            mli.methods.emplace_back(mi);
        }

        m_info->classes.emplace_back(ci);
        m_info->methodLists[mli.address] = mli;
    }
}
