//
// Created by mikag on 10/10/2018.
//

#ifndef RELIC_2_0_RESOURCEMANAGER_H
#define RELIC_2_0_RESOURCEMANAGER_H

#include <cstdint>
#include <cstdio>
#include <map>
#include <Core/RelicStruct.h>
#include <Importers/IImporter.h>
#include "Compression/CompressionManager.h"

class ResourceManager
{
private:
    CompressionManager manager;
    std::map<uint_fast32_t, void*> resources;

    static ResourceManager * instance;
public:
    template<typename T>
    T * GetSimpleResourceData(uint_fast32_t guid);
    void * GetResourceData(uint_fast64_t guid);
    bool IsResourceLoaded(uint_fast32_t guid);
    void SetResourceData(uint_fast32_t guid, RelicType type, size_t size, void *data, bool write = true);
    void SetRPACK(std::string rpack, bool deleteResources = false);

    static ResourceManager * GetInstance();

    ResourceManager();
    ~ResourceManager();
};

template<typename T>
T * ResourceManager::GetSimpleResourceData(uint_fast32_t guid)
{
    if(!manager.HasRPACKLoaded())
    {
        Logger::Log("[ResourceManager] Cannot load resource when no RPACK is loaded.");
        return nullptr;
    }

    auto resource = resources.find(guid);
    if(resource != resources.end())
    {
        return resource->second;
    }

    auto * res =  manager.LoadResource<T>(guid);
    resources.insert(std::pair<uint_fast32_t, void *>(guid, res));
    return res;
}



#endif //RELIC_2_0_RESOURCEMANAGER_H
