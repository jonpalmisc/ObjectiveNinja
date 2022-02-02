/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include "BinaryNinja.h"

/**
 * Plugin commands.
 */
class Commands {
public:
    /**
     * Define types for Objective-C structures.
     */
    static void defineTypes(BinaryViewRef);

    /**
     * Analyze all Objective-C structures in the binary.
     */
    static void analyzeStructures(BinaryViewRef);

    /**
     * Register plugin commands for all one-shot actions.
     */
    static void registerCommands();
};
