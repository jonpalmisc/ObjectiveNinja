/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include "AbstractFile.h"
#include "AnalysisInfo.h"

#include <memory>

namespace ObjectiveNinja {

using SharedAnalysisInfo = std::shared_ptr<AnalysisInfo>;
using SharedAbstractFile = std::shared_ptr<AbstractFile>;

/**
 * Bitmask used to remove the tags from a tagged pointer.
 */
constexpr uint64_t PointerMask = 0xFFFFFFFFF;

/**
 * Abstract base class for analyzers.
 */
class Analyzer {
protected:
    std::shared_ptr<AnalysisInfo> m_info;
    std::shared_ptr<AbstractFile> m_file;

    /**
     * Automatically resolve a pointer.
     *
     * This procedure supports pointers that are
     *
     *   1. direct and untagged (x86_64);
     *   2. direct and tagged (macOS 11, arm64e); and
     *   3. direct and image-relative (macOS 12+, arm64e).
     */
    uint64_t arp(uint64_t pointer) const
    {
        pointer &= PointerMask;
        if (!pointer)
            return 0;

        // If the pointer --- after removing the tags --- is greater than the
        // image base, it is likely a direct pointer.
        if (pointer > m_file->imageBase()) {
            return pointer;
        }

        // Otherwise, it is likely to be an offset from the image base, meaning
        // the absolute pointer needs to be calculated.
        return pointer + m_file->imageBase();
    }

public:
    Analyzer(SharedAnalysisInfo, SharedAbstractFile);
    virtual ~Analyzer() = default;

    virtual void run() = 0;
};

}
