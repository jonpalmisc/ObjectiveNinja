/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include <ObjectiveNinjaCore/AbstractFile.h>

#include <binaryninjaapi.h>

using BinaryViewRef = BinaryNinja::Ref<BinaryNinja::BinaryView>;

class BinaryViewFile : public ObjectiveNinja::AbstractFile {
    BinaryViewRef m_bv;
    BinaryNinja::BinaryReader m_reader;

public:
    explicit BinaryViewFile(BinaryViewRef);

    void seek(uint64_t) override;

    uint8_t readByte() override;
    uint32_t readInt() override;
    uint64_t readLong() override;

    uint64_t imageBase() const override;
    uint64_t sectionStart(const std::string& name) const override;
    uint64_t sectionEnd(const std::string& name) const override;
};
