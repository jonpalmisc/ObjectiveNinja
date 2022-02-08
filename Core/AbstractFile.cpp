/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include <ObjectiveNinjaCore/AbstractFile.h>

namespace ObjectiveNinja {

uint8_t AbstractFile::readByte(uint64_t offset)
{
    seek(offset);
    return readByte();
}

uint32_t AbstractFile::readInt(uint64_t offset)
{
    seek(offset);
    return readInt();
}

uint64_t AbstractFile::readLong(uint64_t offset)
{
    seek(offset);
    return readLong();
}

std::string AbstractFile::readString(size_t maxLength)
{
    std::string result;

    while (maxLength == 0 || result.size() <= maxLength) {
        char c = static_cast<char>(readByte());

        if (c == 0)
            break;

        result += c;
    }

    return result;
}

std::string AbstractFile::readStringAt(uint64_t address, size_t maxLength)
{
    seek(address);
    return readString();
}

}
