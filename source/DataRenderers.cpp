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

bool TaggedPointerDataRenderer::IsValidForData(BinaryView* bv, uint64_t,
    Type* type, std::vector<std::pair<Type*, size_t>>&)
{
    auto taggedPointerType = bv->GetTypeByName(CustomTypes::TaggedPointer);
    if (!taggedPointerType)
        return false;

    return type->GetRegisteredName() == taggedPointerType->GetRegisteredName();
}

std::vector<DisassemblyTextLine> TaggedPointerDataRenderer::GetLinesForData(
    BinaryView* bv, uint64_t addr, Type*,
    const std::vector<InstructionTextToken>& prefix, size_t,
    std::vector<std::pair<Type*, size_t>>&)
{
    BinaryReader reader(bv);
    reader.Seek(addr);

    auto pointer = reader.Read64() & 0xFFFFFFFF;
    if (bv->GetDefaultArchitecture()->GetName() == "aarch64" && pointer != 0)
        pointer += bv->GetStart();

    std::string tokenText;
    BNInstructionTextTokenType tokenType;

    // If this tagged pointer points to a symbol, use its name as the token
    // text, otherwise format the address it points to as a hex string.
    Ref<Symbol> symbol = bv->GetSymbolByAddress(pointer);
    if (symbol) {
        tokenText = symbol->GetFullName();
        tokenType = DataSymbolToken;
    } else {
        char addressBuffer[32];
        sprintf(addressBuffer, "0x%llx", pointer);

        tokenText = std::string(addressBuffer);
        tokenType = CodeRelativeAddressToken;
    }

    // Create a line using the prefix tokens passed in.
    DisassemblyTextLine line;
    line.addr = addr;
    line.tokens = prefix;
    line.tokens.emplace_back(tokenType, tokenText, pointer);

    return { line };
}

void TaggedPointerDataRenderer::Register()
{
    DataRendererContainer::RegisterTypeSpecificDataRenderer(new TaggedPointerDataRenderer());
}
