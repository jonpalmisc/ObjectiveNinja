/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include <ObjectiveNinjaCore/ABI.h>

namespace ObjectiveNinja::ABI {

uint64_t decodePointer(uint64_t pointer, uint64_t imageBase)
{
    pointer &= PointerMask;
    if (!pointer)
        return 0;

    // If the pointer --- after removing the tags --- is greater than the
    // image base, it is likely a direct pointer.
    if (pointer > imageBase) {
        return pointer;
    }

    // Otherwise, it is likely to be an offset from the image base, meaning
    // the absolute pointer needs to be calculated.
    return pointer + imageBase;
}

}
