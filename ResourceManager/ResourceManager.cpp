//
// Created by mikag on 10/10/2018.
//

#include "ResourceManager.h"

//TODO: Actually implement this.
void* ResourceManager::GetResourceData(uint_fast32_t guid, size_t *size)
{
    int* resource = new int();
    *resource = 4;
    *size = sizeof(*resource);
    return resource;
}
