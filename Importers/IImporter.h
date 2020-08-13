//
// Created by mikag on 4/1/2020.
//

#ifndef RELIC_IIMPORTER_H
#define RELIC_IIMPORTER_H

#include <string>
#include <map>
#include <typeindex>
#include <Core/RelicStruct.h>
#include "ImportUtil.h"

typedef class IImporter * (*IImporterGetter)();

class IImporter
{
private:
    //Find out if there's a better way to do this without having to make this a function
    //without relying upon static initialisation to go in a certain order.
    static std::map<RelicType, IImporterGetter> &registry();

protected:
    static void ReadBin(void* data, void* dest, size_t& offset, size_t size);

public:
    virtual GUID ImportResource(const std::string & filePath) = 0;
    virtual void * Deserialize(void* data, size_t dataSize) = 0;
    virtual void * Serialize(void * resource, size_t & totalSize) = 0;
    static IImporter * GetImporterForType(RelicType type);
    static void RegisterImporter(RelicType type, IImporterGetter getter);
};

struct ImporterRegistrar
{
    ImporterRegistrar(RelicType type, IImporterGetter getter)
    {
        IImporter::RegisterImporter(type, getter);
    }
};

#endif //RELIC_IIMPORTER_H
