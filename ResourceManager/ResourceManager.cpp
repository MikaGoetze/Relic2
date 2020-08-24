//
// Created by mikag on 10/10/2018.
//

#include "ResourceManager.h"

#include <utility>

void ResourceManager::SetResourceData(uint_fast32_t guid, RelicType type, size_t size, void *data, bool write)
{
    if (!manager.HasRPACKLoaded())
    {
        SetRPACK("default.rpack");
    }

    //TODO: Batch this up
    IImporter *importer = IImporter::GetImporterForType(type);
    if (importer == nullptr)
    {
        manager.AddResource(data, size, guid, type);
    } else
    {
        size_t serializedDataSize;
        void *serializedData = importer->Serialize(data, serializedDataSize);
        if (serializedData != nullptr) manager.AddResource(serializedData, serializedDataSize, guid, type);
    }

    if (write) manager.WriteRPACK();

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

void *ResourceManager::GetResourceData(uint_fast32_t guid, bool forceReload)
{
    if (!forceReload)
    {
        auto resource = resources.find(guid);
        if (resource != resources.end())
        {
            return resource->second;
        }
    }

    size_t resourceSize;
    RelicType type = REL_TYPE_NONE;
    void *data = manager.LoadResourceBinary(guid, resourceSize, type);

    //Now we need to import the binary data.
    IImporter *importer = IImporter::GetImporterForType(type);
    if (importer == nullptr)
    {
        //Then we assume it's just raw data.
        Logger::Log("[ResourceManager] [WRN] Could not find importer for type %s assuming no import is needed.", std::to_string(type).c_str());
        return data;
    }

    Logger::Log("%s", std::to_string(resourceSize).c_str());
    Logger::Log("%s", std::to_string(type).c_str());


    return importer->Deserialize(data, resourceSize);
}

ResourceManager::~ResourceManager()
{
    instance = nullptr;
}

void ResourceManager::SetRPACK(std::string rpack, bool deleteResources)
{
    manager.SetRPACK(std::move(rpack), deleteResources);
}

void ResourceManager::WriteRPACK()
{
    manager.WriteRPACK();
}

GUID ResourceManager::ImportResource(std::string filepath, RelicType type)
{
    IImporter *importer = IImporter::GetImporterForType(type);
    return importer->ImportResource(filepath);
}



ResourceManager *ResourceManager::instance;
