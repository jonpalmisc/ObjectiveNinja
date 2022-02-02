/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include "CustomTypes.h"

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
