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

#include "AnalysisRecords.hpp"

#include <binaryninjaapi.h>

using BinaryViewRef = BinaryNinja::Ref<BinaryNinja::BinaryView>;
using TypeRef = BinaryNinja::Ref<BinaryNinja::Type>;

/// Collection of Objective-C section name constants.
namespace SectionName {

constexpr auto CFString = "__cfstring";
constexpr auto ClassList = "__objc_classlist";
constexpr auto SelectorRefs = "__objc_selrefs";

}

/// Analyzer for Objective-C structures.
class StructureAnalyzer {
    BinaryViewRef m_bv;
    uint64_t m_imageBase;

    BinaryNinja::BinaryReader m_reader;
    BinaryNinja::BinaryWriter m_writer;

    bool m_isARM64;

    TypeRef m_cfStringType;
    TypeRef m_methodType;
    TypeRef m_methodListType;
    TypeRef m_classDataType;
    TypeRef m_classType;

    AnalysisRecords m_records;

    /// Mask used to remove flags in high bits of addresses and offsets.
    static constexpr uint64_t OffsetMask = 0xFFFFFFFFF;

    StructureAnalyzer(BinaryViewRef);

    /// Seek the internal reader and writer to the same address.
    void seek(uint64_t address)
    {
        m_reader.Seek(address);
        m_writer.Seek(address);
    }

    /// Read and decode an encoded pointer.
    ///
    /// Reads a (potentially-encoded) pointer from the current internal reader
    /// position and applies the appropriate decoding procedure depending on the
    /// architecture of the binary. Optionally, overwrite the encoded pointer
    /// with its decoded value.
    ///
    /// \param fix If true, write the decoded pointer back to the binary.
    uint64_t readEncodedPointer(bool fix = true);

    /// Define a symbol with a prefix and (optional) label.
    ///
    /// If no label is provided, the address will be formatted in hexadecimal
    /// and used in place of the label.
    void defineSymbol(uint64_t, const std::string& prefix,
        const std::string& label = "");

    /// Create a string data variable and return the content of the string.
    std::string defineStringData(uint64_t);

    /// Analyze a CFString.
    void analyzeCFString(uint64_t);

    /// Analyze a selector reference.
    SelectorRefRecord analyzeSelectorRef(uint64_t);

    /// Analyze a method.
    MethodRecord analyzeMethod(uint64_t);

    /// Analyze a method list.
    MethodListRecord analyzeMethodList(uint64_t);

    /// Analyze a class data (RO) instance.
    ClassDataRecord analyzeClassData(uint64_t);

    /// Analyze a class.
    ClassRecord analyzeClass(uint64_t);

    /// Run all structure analysis procedures.
    void runPrivate();

public:
    /// Run structure analysis and get the analysis records for a view.
    static AnalysisRecords run(BinaryViewRef);
};
