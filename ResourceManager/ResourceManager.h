//
// Created by mikag on 10/10/2018.
//

#ifndef RELIC_2_0_RESOURCEMANAGER_H
#define RELIC_2_0_RESOURCEMANAGER_H

#include <cstdint>
#include <cstdio>

class ResourceManager
{
private:
public:
    void* GetResourceData(uint_fast32_t guid, size_t* size);
};


#endif //RELIC_2_0_RESOURCEMANAGER_H
