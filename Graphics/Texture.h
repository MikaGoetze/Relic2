//
// Created by mikag on 31/08/2020.
//

#ifndef RELIC_TEXTURE_H
#define RELIC_TEXTURE_H

#include <Core/RelicStruct.h>
#include <cstdint>

struct Texture : RelicStruct
{
    uint32_t sType = REL_STRUCTURE_TYPE_TEXTURE;
    unsigned char* data;
    uint32_t width;
    uint32_t height;
    uint32_t numComponents;
    uint32_t dataSize;

    ~Texture()
    {
        delete[] data;
    }
};


#endif //RELIC_TEXTURE_H
