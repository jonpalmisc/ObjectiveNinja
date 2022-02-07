/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#pragma once

#include "BinaryNinja.h"

/**
 * All type-related things.
 */
namespace CustomTypes {

const std::string TaggedPointer = "tptr_t";
const std::string RelativePointer = "rptr_t";

const std::string ID = "id";
const std::string Selector = "SEL";

const std::string CFString = "CFString";

const std::string MethodList = "objc_method_list_t";
const std::string Method = "objc_method_t";
const std::string MethodListEntry = "objc_method_entry_t";
const std::string Class = "objc_class_t";
const std::string ClassRO = "objc_class_ro_t";

/**
 * Define all Objective-C-related types for a view.
 */
void defineAll(BinaryNinja::Ref<BinaryNinja::BinaryView>);

}
