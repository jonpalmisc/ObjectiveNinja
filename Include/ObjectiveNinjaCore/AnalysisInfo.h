/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include "../vendor/json/json.hpp"

namespace ObjectiveNinja {

/**
 * A description of a CFString instance.
 */
struct CFStringInfo {
    uint64_t address {};
    uint64_t dataAddress {};
    size_t size {};
};

/**
 * A description of a selector reference.
 */
struct SelectorRefInfo {
    uint64_t address {};

    std::string name {};

    uint64_t rawSelector {};
    uint64_t nameAddress {};
};

using SharedSelectorRefInfo = std::shared_ptr<SelectorRefInfo>;

/**
 * A description of an Objective-C class.
 */
struct ClassInfo {
    uint64_t address {};

    std::string name {};

    uint64_t listPointer {};
    uint64_t dataAddress {};
    uint64_t nameAddress {};
    uint64_t methodListAddress {};
};

/**
 * A description of an Objective-C method.
 */
struct MethodInfo {
    uint64_t address {};

    uint64_t nameAddress {};
    uint64_t typeAddress {};
    uint64_t implAddress {};
};

/**
 * A description of an Objective-C method list.
 */
struct MethodListInfo {
    uint64_t address {};
    uint32_t flags {};
    std::vector<MethodInfo> methods {};

    /**
     * Tells whether the method list uses relative offsets or not.
     */
    bool hasRelativeOffsets() const;

    /**
     * Tells whether the method list uses direct selectors or not.
     */
    bool hasDirectSelectors() const;
};

/**
 * Analysis info storage.
 *
 * AnalysisInfo is intended to be a common structure for persisting information
 * during and after analysis. All significant info obtained or produced through
 * analysis should be stored here, ideally in the form of other *Info structs.
 */
struct AnalysisInfo {
    std::vector<CFStringInfo> cfStrings {};
    std::unordered_map<uint64_t, SharedSelectorRefInfo> selectorRefs {};
    std::vector<ClassInfo> classes {};
    std::unordered_map<uint64_t, MethodListInfo> methodLists {};
    std::unordered_map<uint64_t, uint64_t> methodImpls;

    std::string dump() const;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CFStringInfo, address, dataAddress, size);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SelectorRefInfo, address, name, rawSelector,
    nameAddress);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClassInfo, listPointer, address, dataAddress,
    nameAddress, name, methodListAddress);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MethodInfo, address, nameAddress,
    typeAddress, implAddress);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MethodListInfo, address, flags, methods);

}
