//
// Created by mikag on 10/10/2018.
//

#include "ResourceManager.h"

void ResourceManager::SetResourceData(uint_fast32_t guid, RelicType type, size_t size, void *data, bool write)
{
    if(!manager.HasRPACKLoaded())
    {
        Logger::Log("[ResourceManager] Please set an RPACK before trying to set resource data.");
        return;
    }

    //TODO: Batch this up
    IImporter * importer = IImporter::GetImporterForType(type);
    if(importer == nullptr)
    {
        manager.AddResource(data, size, guid, type);
    }
    else
    {
        size_t serializedDataSize;
        void * serializedData = importer->Serialize(data, serializedDataSize);
        manager.AddResource(serializedData, serializedDataSize, guid, type);
    }

    if(write) manager.WriteRPACK();

    resources.insert_or_assign(guid, data);
}

bool ResourceManager::IsResourceLoaded(uint_fast32_t guid)
{
    return resources.find(guid) != resources.end();
}

ResourceManager *ResourceManager::GetInstance()
{
    return instance;
}

ResourceManager::ResourceManager()
{
    instance = this;
}

void *ResourceManager::GetResourceData(uint_fast64_t guid)
{
    auto resource = resources.find(guid);
    if(resource != resources.end())
    {
        return resource->second;
    }


    size_t resourceSize;
    RelicType type = REL_TYPE_NONE;
    void* data = manager.LoadResourceBinary(guid, resourceSize, type);

    //Now we need to import the binary data.
    IImporter * importer = IImporter::GetImporterForType(type);
    if(importer == nullptr)
    {
        //Then we assume it's just raw data.
        Logger::Log(3, "[ResourceManager] [WRN] Could not find importer for type ", std::to_string(type).c_str(), " assuming no import is needed.");
        return data;
    }

    Logger::Log(std::to_string(resourceSize).c_str());
    Logger::Log(type);


    return importer->Deserialize(data, resourceSize);
}

ResourceManager::~ResourceManager()
{
    instance = nullptr;
}

void ResourceManager::SetRPACK(std::string rpack, bool deleteResources)
{
    manager.SetRPACK(rpack, deleteResources);
}

ResourceManager * ResourceManager::instance;
