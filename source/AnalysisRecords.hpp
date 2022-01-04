/*
 * Copyright (c) 2022 Jon Palmisciano
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

/// Selector reference record structure.
struct SelectorRefRecord {
    /// Address where the selector reference is defined.
    uint64_t address {};

    /// Raw value (with flags) of the selector.
    uint64_t selector {};

    /// Address of the selector's name string.
    uint64_t nameAddress {};
};

/// Method list record structure.
struct MethodRecord {
    /// Address of the method structure.
    uint64_t address {};

    /// Address of the related selector reference or method name string.
    uint64_t nameAddress {};

    /// Address of the method's implementation.
    uint64_t impAddress {};

    /// Name of the method; only populated post-analysis.
    std::string name;
};

/// Method list record structure.
struct MethodListRecord {
    /// Address of the method list structure.
    uint64_t address {};

    /// All methods belonging to the method list.
    std::vector<MethodRecord> methods;
};

/// Class data record structure.
struct ClassDataRecord {
    /// Address of the class data structure.
    uint64_t address {};

    /// Address of the class name string.
    uint64_t nameAddress {};

    /// Name of the class; only populated post-analysis.
    std::string name;

    /// Method list containing the class's methods.
    MethodListRecord methodList;
};

/// Class record structure.
struct ClassRecord {
    /// Address of the pointer to this class in the class list section.
    uint64_t pointerAddress {};

    /// Address of the class structure.
    uint64_t address {};

    /// Associated class data record.
    ClassDataRecord data;
};

/// Record storage interface for data collected during analysis.
struct AnalysisRecords {
    AnalysisRecords() = default;

    /// All classes indexed during analysis.
    std::vector<ClassRecord> classes;

    /// All selector references indexed during analysis.
    std::unordered_map<uint64_t, SelectorRefRecord> selectorRefs;

    /// Map of selector name addresses to method implementations.
    std::unordered_map<uint64_t, uint64_t> impMap;

    /// List of all known method implementation functions.
    std::set<uint64_t> imps;
};
