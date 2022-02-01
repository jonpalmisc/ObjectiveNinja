/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include <cstdint>
#include <string>

namespace ObjectiveNinja {

/**
 * A common interface to wrap a file (or another data source) for reading.
 *
 * AbstractFile declares numerous APIs that must be implemented by consumers of
 * the ObjectiveNinja core library. All methods must have a valid
 * implementation in order for analysis to function correctly.
 */
class AbstractFile {
public:
    /**
     * Seek the reader to the given offset.
     */
    virtual void seek(uint64_t) = 0;

    /**
     * Read a byte from the current reader offset.
     */
    virtual uint8_t readByte() = 0;

    /**
     * Read a byte at the given offset.
     */
    uint8_t readByte(uint64_t offset);

    /**
     * Read a 32-bit integer at the current reader offset.
     */
    virtual uint32_t readInt() = 0;

    /**
     * Read a 32-bit integer at the given offset.
     */
    uint32_t readInt(uint64_t offset);

    /**
     * Read a 64-bit integer at the current reader offset.
     */
    virtual uint64_t readLong() = 0;

    /**
     * Read a 64-bit integer at the given offset.
     */
    uint64_t readLong(uint64_t offset);

    /**
     * Read a string starting at the current reader offset. If no max length is
     * specified, a null-terminated string will be read.
     */
    std::string readString(size_t maxLength = 0);

    /**
     * Read a string starting at the given offset. If no max length is
     * specified, a null-terminated string will be read.
     */
    std::string readString(uint64_t, size_t maxLength = 0);

    /**
     * Get the base offset of the image/file.
     */
    virtual uint64_t imageBase() const = 0;

    /**
     * Get the offset corresponding to the start of the given section.
     */
    virtual uint64_t sectionStart(const std::string&) const = 0;

    /**
     * Get the offset corresponding to the end of the given section.
     */
    virtual uint64_t sectionEnd(const std::string&) const = 0;
};

}
