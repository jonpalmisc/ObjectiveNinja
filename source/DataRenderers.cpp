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

#include "DataRenderers.hpp"

#include "CustomTypes.hpp"

#include <cstdio>

using namespace BinaryNinja;

/// Get the appropriate token type for a pointer to a given symbol.
BNInstructionTextTokenType tokenTypeForSymbol(Ref<Symbol> symbol)
{
    auto tokenType = CodeRelativeAddressToken;

    switch (symbol->GetType()) {
    case DataSymbol:
        tokenType = DataSymbolToken;
        break;
    case FunctionSymbol:
        tokenType = CodeSymbolToken;
        break;
    default:
        break;
    }

    return tokenType;
}

/// Get a line for a given pointer.
DisassemblyTextLine lineForPointer(BinaryView* bv, uint64_t pointer,
    uint64_t addr, const std::vector<InstructionTextToken>& prefix)
{
    std::string tokenText = "???";
    auto tokenType = CodeRelativeAddressToken;

    Ref<Symbol> symbol = bv->GetSymbolByAddress(pointer);
    if (pointer == 0 || pointer == bv->GetStart()) {
        tokenText = "NULL";
        tokenType = KeywordToken;
    } else if (symbol) {
        tokenText = symbol->GetFullName();
        tokenType = tokenTypeForSymbol(symbol);
    } else {
        char addressBuffer[32];
        sprintf(addressBuffer, "0x%llx", pointer);

        tokenText = std::string(addressBuffer);
        tokenType = CodeRelativeAddressToken;
    }

    DisassemblyTextLine line;
    line.addr = addr;
    line.tokens = prefix;
    line.tokens.emplace_back(tokenType, tokenText, pointer);

    return { line };
}

// Tells if a type is a given type.
bool isType(BinaryView* bv, Type* type, const std::string& name)
{
    auto targetType = bv->GetTypeByName(name);
    if (!targetType)
        return false;

    return type->GetRegisteredName() == targetType->GetRegisteredName();
}

bool TaggedPointerDataRenderer::IsValidForData(BinaryView* bv, uint64_t,
    Type* type, std::vector<std::pair<Type*, size_t>>&)
{
    return isType(bv, type, CustomTypes::TaggedPointer);
}

std::vector<DisassemblyTextLine> TaggedPointerDataRenderer::GetLinesForData(
    BinaryView* bv, uint64_t addr, Type*,
    const std::vector<InstructionTextToken>& prefix, size_t,
    std::vector<std::pair<Type*, size_t>>&)
{
    BinaryReader reader(bv);
    reader.Seek(addr);

    auto pointer = (reader.Read64() & 0xFFFFFFFF) + bv->GetStart();

    return { lineForPointer(bv, pointer, addr, prefix) };
}

void TaggedPointerDataRenderer::Register()
{
    DataRendererContainer::RegisterTypeSpecificDataRenderer(new TaggedPointerDataRenderer());
}

bool RelativePointerDataRenderer::IsValidForData(BinaryView* bv, uint64_t,
    Type* type, std::vector<std::pair<Type*, size_t>>&)
{
    return isType(bv, type, CustomTypes::RelativePointer);
}

std::vector<DisassemblyTextLine> RelativePointerDataRenderer::GetLinesForData(
    BinaryView* bv, uint64_t addr, Type*,
    const std::vector<InstructionTextToken>& prefix, size_t,
    std::vector<std::pair<Type*, size_t>>&)
{
    BinaryReader reader(bv);
    reader.Seek(addr);

    auto pointer = (int32_t)reader.Read32() + addr;

    return { lineForPointer(bv, pointer, addr, prefix) };
}

void RelativePointerDataRenderer::Register()
{
    DataRendererContainer::RegisterTypeSpecificDataRenderer(new RelativePointerDataRenderer());
}
