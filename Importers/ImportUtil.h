//
// Created by mikag on 3/29/2020.
//

#ifndef RELIC_IMPORTUTIL_H
#define RELIC_IMPORTUTIL_H

#include <cstdint>
#include <string>
#define GUID uint_fast32_t
#define GUID_INVALID 0

static inline uint32_t murmur_32_scramble(uint32_t k);

uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed);

GUID GetGUID(std::string resourceName);

#endif //RELIC_IMPORTUTIL_H
