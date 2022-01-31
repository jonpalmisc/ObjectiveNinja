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

#include "CustomTypes.hpp"

constexpr const char* AllTypesSource = R"(
struct tptr_t {
    uint64_t raw;
};

struct rptr_t {
    int32_t raw;
};

typedef void* id;
typedef char* SEL;

struct CFString {
    const tptr_t isa;
    uint64_t flags;
    const tptr_t data;
    uint64_t size;
};

struct objc_method_entry_t {
    rptr_t name;
    rptr_t types;
    rptr_t imp;
};

struct objc_method_t {
    tptr_t name;
    tptr_t types;
    tptr_t imp;
};

struct objc_method_list_t {
    uint32_t obsolete;
    uint32_t count;
};

struct objc_class_ro_t {
    uint32_t flags;
    uint32_t start;
    uint32_t size;
    uint32_t reserved;
    const tptr_t ivar_layout;
    const tptr_t name;
    const tptr_t methods;
    const tptr_t protocols;
    const tptr_t vars;
    const tptr_t weak_ivar_layout;
    const tptr_t properties;
};

struct objc_class_t {
    const tptr_t isa;
    const tptr_t super;
    void* cache;
    void* vtable;
    const tptr_t data;
};
)";

namespace CustomTypes {

using namespace BinaryNinja;

void defineAll(Ref<BinaryView> bv)
{
    std::map<QualifiedName, Ref<Type>> types, variables, functions;
    std::string errors;

    bv->GetDefaultPlatform()->ParseTypesFromSource(AllTypesSource,
        "ObjectiveNinja.h", types, variables, functions, errors);

    for (const auto& [name, type] : types)
        bv->DefineUserType(name, type);
}

}
