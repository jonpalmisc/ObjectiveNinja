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

#include "StructureAnalyzer.hpp"
#include "CustomTypes.hpp"

#include <cinttypes>

using namespace BinaryNinja;

StructureAnalyzer::StructureAnalyzer(BinaryViewRef bv)
    : m_bv(bv)
    , m_imageBase(bv->GetStart())
    , m_reader(bv)
    , m_writer(bv)
    , m_isARM64(bv->GetDefaultArchitecture()->GetName() == "aarch64")
    , m_cfStringType(Type::NamedType(m_bv, CustomTypes::CFString))
    , m_methodListType(Type::NamedType(m_bv, CustomTypes::MethodList))
    , m_classDataType(Type::NamedType(m_bv, CustomTypes::ClassData))
    , m_classType(Type::NamedType(m_bv, CustomTypes::Class))
{
    if (m_isARM64)
        m_methodType = Type::NamedType(m_bv, CustomTypes::SmallMethod);
    else
        m_methodType = Type::NamedType(m_bv, CustomTypes::Method);
}

uint64_t StructureAnalyzer::readEncodedPointer(bool fix)
{
    auto pointer = m_reader.Read64() & OffsetMask;

    // On x86_64, the value in `pointer` at this point should be a valid
    // pointer, as absolute addresses are used in encoding. However, on ARM64,
    // pointers are encoded as an offset from the image base, which must be
    // added to the offset to produce the correct pointer.
    if (m_isARM64 && pointer != 0)
        pointer += m_imageBase;

    // Write back the decoded pointer if requested.
    if (fix)
        m_writer.Write64(pointer);

    return pointer;
}

void StructureAnalyzer::defineSymbol(uint64_t address, const std::string& prefix,
    const std::string& label)
{
    const auto formatAddress = [](uint64_t address) {
        char buffer[32];
        std::sprintf(buffer, "%" PRIx64, address);

        return std::string(buffer);
    };

    auto text = prefix + "_";
    if (!label.empty())
        text += label;
    else
        text += formatAddress(address);

    auto* symbol = new Symbol(DataSymbol, text, address);
    m_bv->DefineUserSymbol(symbol);
}

std::string StructureAnalyzer::defineStringData(uint64_t start)
{
    std::string text;
    size_t size = 0;

    m_reader.Seek(start);
    auto c = m_reader.Read8();
    while (c != 0) {
        text += static_cast<char>(c);
        ++size;

        c = m_reader.Read8();
    }

    m_bv->DefineDataVariable(start, Type::ArrayType(Type::IntegerType(1, true), ++size));
    return text;
}

//  Offset  Type           Description
// ---------------------------------------
//    +0x0  objc_class_t*  Class pointer
//    +0x8  uint64_t       Flags
//   +0x10  const char*    String pointer
//   +0x18  uint64_t       String length
//
void StructureAnalyzer::analyzeCFString(uint64_t address)
{
    seek(address + 0x10);
    const auto dataAddress = readEncodedPointer();
    defineStringData(dataAddress);

    m_bv->DefineDataVariable(address, m_cfStringType);
}

SelectorRefRecord StructureAnalyzer::analyzeSelectorRef(uint64_t address)
{
    seek(address);
    const auto rawSelector = m_reader.Read64();

    seek(address);
    const auto nameAddress = readEncodedPointer();

    m_bv->DefineDataVariable(address,
        Type::PointerType(8, Type::NamedType(m_bv, CustomTypes::Selector)));
    return { address, rawSelector, nameAddress };
}

uint64_t StructureAnalyzer::analyzeClassRef(uint64_t address)
{
    seek(address);
    const auto rawRef = m_reader.Read64();

    // This expression will be true if this is a class reference that points
    // outside the binary. In those cases, there is nothing that can be done.
    if (rawRef & 0x8000000000000000)
        return 0;

    seek(address);
    const auto classAddress = readEncodedPointer();

    m_bv->DefineDataVariable(address,
        Type::PointerType(8, Type::NamedType(m_bv, CustomTypes::Class)));
    return classAddress;
}

//  Offset  Type         Description
// ----------------------------------------------------
//    +0x0  const char*  Name pointer/offset
//    +0x4  void*        Type info pointer/offset
//    +0x8  void*        Implementation pointer/offset
//
// On ARM64, the values are relative offsets. On x86_64, they are just normal
// pointers, and every value in the offset column should be doubled.
MethodRecord StructureAnalyzer::analyzeMethod(uint64_t address)
{
    seek(address);

    uint64_t nameAddress = 0, impAddress = 0;
    if (m_isARM64) {
        nameAddress = address + (int32_t)m_reader.Read32();
        m_reader.Read32();
        impAddress = address + 8 + (int32_t)m_reader.Read32();
    } else {
        nameAddress = readEncodedPointer();
        readEncodedPointer();
        impAddress = readEncodedPointer();
    }

    m_bv->DefineDataVariable(address, m_methodType);
    return MethodRecord { address, nameAddress, impAddress, "???" };
}

//  Offset  Type      Description
// ------------------------------------
//    +0x0  uint32_t  Obsolete/unused?
//    +0x4  uint32_t  Method count
//
MethodListRecord StructureAnalyzer::analyzeMethodList(uint64_t address)
{
    seek(address);

    m_reader.Read32();
    const auto methodCount = m_reader.Read32();
    const auto methodSize = m_methodType->GetWidth();

    std::vector<MethodRecord> methods;
    methods.reserve(methodCount);
    for (unsigned i = 0; i < methodCount; ++i) {
        auto methodAddress = address + 8 + (i * methodSize);

        auto method = analyzeMethod(methodAddress);
        methods.emplace_back(method);
    }

    m_bv->DefineDataVariable(address, m_methodListType);
    return { address, methods };
}

//  Offset  Type                 Description
// --------------------------------------------------
//   +0x18  const char*          Class name pointer
//   +0x20  objc_method_list_t*  Method list pointer
//
// See full listing in CustomTypes.cpp, some fields omitted for brevity.
ClassDataRecord StructureAnalyzer::analyzeClassData(uint64_t address)
{
    seek(address + 0x18);

    const auto nameAddress = readEncodedPointer();
    const auto methodListAddress = readEncodedPointer();

    MethodListRecord methodList;
    if (methodListAddress)
        methodList = analyzeMethodList(methodListAddress);

    m_bv->DefineDataVariable(address, m_classDataType);
    return { address, nameAddress, "???", methodList };
}

//  Offset  Type              Description
// -------------------------------------------------------------
//    +0x0  objc_class_t*     Class pointer
//    +0x8  objc_class_t*     Superclass pointer
//   +0x10  void*             Cache pointer? (currently unused)
//   +0x18  void*             VMT pointer? (currently unused)
//   +0x20  objc_class_ro_t*  Class data pointer
//
ClassRecord StructureAnalyzer::analyzeClass(uint64_t address)
{
    seek(address);

    // This address is always encoded as an absolute address, regardless of
    // architecture, therefore `readEncodedPointer()` should not be used here.
    const auto isaAddress = m_reader.Read64() & OffsetMask;
    if (isaAddress)
        m_writer.Write64(isaAddress);

    seek(address + 0x20);
    const auto dataAddress = readEncodedPointer();
    if (!dataAddress)
        return { address, 0, {} };

    const auto classData = analyzeClassData(dataAddress);

    m_bv->DefineDataVariable(address, m_classType);
    return { 0, address, classData };
}

void StructureAnalyzer::runPrivate()
{
    const auto cfStringsSection = m_bv->GetSectionByName(SectionName::CFString);
    if (cfStringsSection) {
        const auto cfStringsStart = cfStringsSection->GetStart();
        const auto cfStringsEnd = cfStringsStart + cfStringsSection->GetLength();
        const auto cfStringSize = m_cfStringType->GetWidth();

        for (auto a = cfStringsStart; a < cfStringsEnd; a += cfStringSize)
            analyzeCFString(a);
    }

    // ---

    const auto classRefsSection = m_bv->GetSectionByName(SectionName::ClassRefs);
    if (classRefsSection) {
        const auto classRefsStart = classRefsSection->GetStart();
        const auto classRefsEnd = classRefsStart + classRefsSection->GetLength();

        for (auto a = classRefsStart; a < classRefsEnd; a += 8) {
            const auto classAddress = analyzeClassRef(a);

            // The call above will return 0 sometimes, such as when the class
            // reference points outside the binary.
            if (classAddress)
                m_records.reverseClassRefs[classAddress] = a;
        }
    }

    // ---

    const auto selRefsSection = m_bv->GetSectionByName(SectionName::SelectorRefs);
    if (!selRefsSection) {
        LogError("Cannot analyze Objective-C selectors; missing  __objc_selrefs section");
        return;
    }

    m_records.selectorRefs.reserve(selRefsSection->GetLength() / 8);

    const auto selRefsStart = selRefsSection->GetStart();
    const auto selRefsEnd = selRefsStart + selRefsSection->GetLength();
    for (auto address = selRefsStart; address < selRefsEnd; address += 8) {
        const auto selectorRef = analyzeSelectorRef(address);
        m_records.selectorRefs.insert({ address, selectorRef });
    }

    // ---

    const auto classListSection = m_bv->GetSectionByName(SectionName::ClassList);
    if (!classListSection) {
        LogError("Cannot analyze Objective-C classes; no class list section present");
        return;
    }

    m_records.classes.reserve(classListSection->GetLength() / 8);

    const auto classListStart = classListSection->GetStart();
    const auto classListEnd = classListStart + classListSection->GetLength();
    for (auto address = classListStart; address < classListEnd; address += 8) {
        m_bv->DefineDataVariable(address, Type::PointerType(8, m_classType));

        seek(address);
        const auto classAddress = readEncodedPointer();
        if (!classAddress)
            continue;

        auto classRecord = analyzeClass(classAddress);
        classRecord.pointerAddress = address;
        m_records.classes.emplace_back(classRecord);
    }

    // ---

    // To keep the structure analysis code cleaner, symbols for structures,
    // methods, etc. are not defined until after all structure analysis is
    // complete. This keeps the analysis code cleaner, and is actually easier
    // than trying to pass additional state between each analysis method.
    for (auto& c : m_records.classes) {
        if (c.data.nameAddress)
            c.data.name = defineStringData(c.data.nameAddress);

        // Define symbols based on the class's name (if applicable) for:
        //
        //  1. the pointer to the class in the `__objc_classlist` section;
        //  2. the class structure itself;
        //  3. the underlying data structure;
        //  4. the class name string;
        //  5. the class's associated method list; and
        //  6. the pointer to the class in the `__objc_classrefs` section.
        //
        defineSymbol(c.pointerAddress, "ptr", c.data.name);
        defineSymbol(c.address, "cls", c.data.name);
        defineSymbol(c.data.address, "ro", c.data.name);
        defineSymbol(c.data.nameAddress, "nm", c.data.name);

        if (c.data.methodList.address)
            defineSymbol(c.data.methodList.address, "ml", c.data.name);

        const auto refAddress = m_records.reverseClassRefs[c.address];
        if (refAddress)
            defineSymbol(refAddress, "ref", c.data.name);

        for (const auto& m : c.data.methodList.methods) {
            std::string selectorName = "???";
            uint64_t selectorNameAddress = 0;

            // On x86_64, the method's name address field actually points to its
            // name, as one would expect. On ARM64, things are a bit different,
            // and the associated selector needs to be queried to get the real
            // address of the method's name.
            if (!m_isARM64) {
                selectorNameAddress = m.nameAddress;
            } else if (m_records.selectorRefs.count(m.nameAddress)) {
                selectorNameAddress = m_records.selectorRefs.at(m.nameAddress).nameAddress;
            }

            // Record the mapping between this selector name address and the
            // implementation, which is used by `Workflow::rewriteMethodCall` to
            // "inline" method calls. The implementation address is also added
            // to a set containing the address of all Objective-C method
            // implementations, which is used in `Workflow::applyBasicTypes` to
            // correct function arguments for Objective-C methods.
            m_records.impMap[selectorNameAddress] = m.impAddress;
            m_records.imps.insert(m.impAddress);

            // TODO: Support alternate "compatibility" naming scheme.
            selectorName = defineStringData(selectorNameAddress);
            const auto methodName = "[" + c.data.name + " " + selectorName + "]";

            // Define a symbol for the method's implementation, indicating the
            // class and selector the implementation corresponds to.
            auto* methodSymbol = new Symbol(FunctionSymbol, methodName, m.impAddress);
            m_bv->DefineUserSymbol(methodSymbol);

            // Define a (not-as-useful) symbol for the method structure itself.
            defineSymbol(m.address, "mth");
        }
    }

    // Write the implementation map to the database for future use.
    std::stringstream stream;
    for (const auto [k, v] : m_records.impMap)
        stream << k << "," << v << ",";

    m_bv->StoreMetadata(MetadataKey::ImpMap, new Metadata(stream.str()));
}

AnalysisRecords StructureAnalyzer::run(BinaryViewRef bv)
{
    StructureAnalyzer analyzer(std::move(bv));
    analyzer.runPrivate();

    return analyzer.m_records;
}
