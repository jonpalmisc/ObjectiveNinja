/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include <chrono>

/**
 * Utilities for measuring performance.
 */
class Performance {
public:
    /**
     * Get the current time.
     */
    static std::chrono::steady_clock::time_point now()
    {
        return std::chrono::high_resolution_clock::now();
    }

    /**
     * Get the current elapsed time from a given start time.
     *
     * Accepts a unit of measure template parameter for the result.
     */
    template <typename T>
    static T elapsed(std::chrono::steady_clock::time_point start)
    {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<T>(end - start);
    }
};
