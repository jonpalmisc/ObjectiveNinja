/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#ifdef ONC_BN_SUPPORT

#include "BinaryViewFile.h"

namespace ObjectiveNinja {

BinaryViewFile::BinaryViewFile(BinaryViewRef bv)
    : m_bv(bv)
    , m_reader(BinaryNinja::BinaryReader(bv))
{
}

void BinaryViewFile::seek(uint64_t address)
{
    m_reader.Seek(address);
}

uint8_t BinaryViewFile::readByte()
{
    return m_reader.Read8();
}

uint32_t BinaryViewFile::readInt()
{
    return m_reader.Read32();
}

uint64_t BinaryViewFile::readLong()
{
    return m_reader.Read64();
}

uint64_t BinaryViewFile::imageBase() const
{
    return m_bv->GetStart();
}

uint64_t BinaryViewFile::sectionStart(const std::string& name) const
{
    auto section = m_bv->GetSectionByName(name);
    if (!section)
        return 0;

    return section->GetStart();
}

uint64_t BinaryViewFile::sectionEnd(const std::string& name) const
{
    auto section = m_bv->GetSectionByName(name);
    if (!section)
        return 0;

    return section->GetStart() + section->GetLength();
}

}

#endif
