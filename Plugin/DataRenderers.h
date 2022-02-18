/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include "BinaryNinja.h"

/**
 * Data renderer for tagged pointers.
 */
class TaggedPointerDataRenderer : public BinaryNinja::DataRenderer {
    TaggedPointerDataRenderer() = default;

public:
    bool IsValidForData(BinaryViewPtr, uint64_t address,
        TypePtr, std::vector<std::pair<TypePtr, size_t>>& context) override;

    std::vector<BinaryNinja::DisassemblyTextLine> GetLinesForData(
        BinaryViewPtr, uint64_t address, TypePtr,
        const std::vector<BinaryNinja::InstructionTextToken>& prefix,
        size_t width, std::vector<std::pair<TypePtr, size_t>>& context) override;

    static void Register();
};

/**
 * Data renderer for fast tagged pointers.
 */
class FastPointerDataRenderer : public BinaryNinja::DataRenderer {
    FastPointerDataRenderer() = default;

public:
    bool IsValidForData(BinaryViewPtr, uint64_t address,
        TypePtr, std::vector<std::pair<TypePtr, size_t>>& context) override;

    std::vector<BinaryNinja::DisassemblyTextLine> GetLinesForData(
        BinaryViewPtr, uint64_t address, TypePtr,
        const std::vector<BinaryNinja::InstructionTextToken>& prefix,
        size_t width, std::vector<std::pair<TypePtr, size_t>>& context) override;

    static void Register();
};

/**
 * Data renderer for relative offset pointers.
 */
class RelativePointerDataRenderer : public BinaryNinja::DataRenderer {
    RelativePointerDataRenderer() = default;

public:
    bool IsValidForData(BinaryViewPtr, uint64_t address,
        TypePtr, std::vector<std::pair<TypePtr, size_t>>& context) override;

    std::vector<BinaryNinja::DisassemblyTextLine> GetLinesForData(
        BinaryViewPtr, uint64_t address, TypePtr,
        const std::vector<BinaryNinja::InstructionTextToken>& prefix,
        size_t width, std::vector<std::pair<TypePtr, size_t>>& context) override;

    static void Register();
};

/**
 * Data renderer for CFString data variables.
 */
class CFStringDataRenderer : public BinaryNinja::DataRenderer {
    CFStringDataRenderer() = default;

public:
    bool IsValidForData(BinaryViewPtr, uint64_t address,
        TypePtr, std::vector<std::pair<TypePtr, size_t>>& context) override;

    std::vector<BinaryNinja::DisassemblyTextLine> GetLinesForData(
        BinaryViewPtr, uint64_t address, TypePtr,
        const std::vector<BinaryNinja::InstructionTextToken>& prefix,
        size_t width, std::vector<std::pair<TypePtr, size_t>>& context) override;

    static void Register();
};
