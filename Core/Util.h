//
// Created by mikag on 9/30/2018.
//

#ifndef RELIC_2_0_UTIL_H
#define RELIC_2_0_UTIL_H

#include <vector>
#include <fstream>

#ifdef _MSC_VER
#include <immintrin.h>
#define PAUSE()\
    _mm_pause()
#else
#define PAUSE()\
    __asm__("pause;");
#endif

static std::vector<char> ReadFile(const std::string& fileName)
{
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if(!file.is_open())
    {
        throw std::runtime_error("failed to open file '" + fileName + "'.");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

#endif //RELIC_2_0_UTIL_H
