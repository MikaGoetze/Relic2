//
// Created by mikag on 3/29/2020.
//

#include "ImportUtil.h"

static inline uint32_t murmur_32_scramble(uint32_t k)
{
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

uint32_t murmur3_32(const uint8_t *key, size_t len, uint32_t seed)
{
    uint32_t h = seed;
    uint32_t k;
/* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--)
    {
// Here is a source of differing results across endiannesses.
// A swap here has no effects on hash properties though.
        k = *((uint32_t *) key);
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
/* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--)
    {
        k <<= 8;
        k |= key[i - 1];
    }
// A swap is *not* necessary here because the preceding loop already
// places the low bytes in the low places according to whatever endianness
// we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur_32_scramble(k);
/* Finalize. */
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

GUID GetGUID(const std::string resourceName)
{
    uint32_t guid = murmur3_32(reinterpret_cast<const uint8_t *>(resourceName.c_str()), resourceName.size(), 0);
    return reinterpret_cast<GUID>(guid);
}
