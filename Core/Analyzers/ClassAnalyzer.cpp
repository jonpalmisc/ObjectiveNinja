/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include <ObjectiveNinjaCore/Analyzers/ClassAnalyzer.h>
#include <ObjectiveNinjaCore/TypeParser.h>

using namespace ObjectiveNinja;

ClassAnalyzer::ClassAnalyzer(SharedAnalysisInfo info,
    SharedAbstractFile file)
    : Analyzer(std::move(info), std::move(file))
{
}

MethodListInfo ClassAnalyzer::analyzeMethodList(uint64_t address)
{
    MethodListInfo mli;
    mli.address = address;
    mli.flags = m_file->readInt(mli.address);

    auto methodCount = m_file->readInt(mli.address + 0x4);
    auto methodSize = mli.hasRelativeOffsets() ? 12 : 24;

    for (unsigned i = 0; i < methodCount; ++i) {
        MethodInfo mi;
        mi.address = mli.address + 8 + (i * methodSize);

        m_file->seek(mi.address);

        if (mli.hasRelativeOffsets()) {
            mi.nameAddress = mi.address + static_cast<int32_t>(m_file->readInt());
            mi.typeAddress = mi.address + 4 + static_cast<int32_t>(m_file->readInt());
            mi.implAddress = mi.address + 8 + static_cast<int32_t>(m_file->readInt());
        } else {
            mi.nameAddress = m_file->readLong();
            mi.typeAddress = m_file->readLong();
            mi.implAddress = m_file->readLong();
        }

        if (!mli.hasRelativeOffsets() || mli.hasDirectSelectors()) {
            mi.selector = m_file->readStringAt(mi.nameAddress);
        } else {
            auto selectorNamePointer = arp(m_file->readLong(mi.nameAddress));
            mi.selector = m_file->readStringAt(selectorNamePointer);
        }

        mi.type = m_file->readStringAt(mi.typeAddress);

        m_info->methodImpls[mi.nameAddress] = mi.implAddress;

        mli.methods.emplace_back(mi);
    }

    return mli;
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
        ci.address = arp(m_file->readLong(address));
        ci.dataAddress = arp(m_file->readLong(ci.address + 0x20));

        // Sometimes the lower two bits of the data address are used as flags
        // for Swift/Objective-C classes. They should be ignored, unless you
        // want incorrect analysis...
        ci.dataAddress &= ~ABI::FastPointerDataMask;

        ci.nameAddress = arp(m_file->readLong(ci.dataAddress + 0x18));
        ci.name = m_file->readStringAt(ci.nameAddress);

        ci.methodListAddress = arp(m_file->readLong(ci.dataAddress + 0x20));
        if (ci.methodListAddress)
            ci.methodList = analyzeMethodList(ci.methodListAddress);

        m_info->classes.emplace_back(ci);
    }
}
