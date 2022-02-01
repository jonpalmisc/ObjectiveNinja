/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include <ObjectiveNinjaCore/AnalysisInfo.h>

#include <binaryninjaapi.h>

using BinaryViewRef = BinaryNinja::Ref<BinaryNinja::BinaryView>;
using TypeRef = BinaryNinja::Ref<BinaryNinja::Type>;
using SymbolRef = BinaryNinja::Ref<BinaryNinja::Symbol>;
using SharedAnalysisInfo = std::shared_ptr<ObjectiveNinja::AnalysisInfo>;

/**
 * Utility class for applying collected AnalysisInfo to a database.
 *
 * InfoHandler is meant to be used after all analyzers intended to run on a
 * database have finished. The resulting AnalysisInfo will then be used to
 * create data variables, symbols, etc. in the database.
 */
class InfoHandler {
    /**
     * Sanitize a string by searching for series of alphanumeric characters and
     * concatenating the matches. The input string will first be truncated.
     */
    static std::string sanitizeText(const std::string&);

    /**
     * Get the type with the given name defined inside the BinaryView.
     */
    static inline TypeRef namedType(BinaryViewRef, const std::string&);

    /**
     * Create a type for a string (character array) of the given size.
     */
    static inline TypeRef stringType(size_t);

    /**
     * Shorthand function for defining a user data variable.
     */
    static inline void defineVariable(BinaryViewRef, uint64_t, TypeRef);

    /**
     * Shorthand function for defining a user symbol, with an optional prefix.
     */
    static inline void defineSymbol(BinaryViewRef, uint64_t,
        const std::string& name, const std::string& prefix = "",
        BNSymbolType type = DataSymbol);

public:
    /**
     * Apply AnalysisInfo to a BinaryView.
     */
    static void applyInfoToView(SharedAnalysisInfo, BinaryViewRef);
};
