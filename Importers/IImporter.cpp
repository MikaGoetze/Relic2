//
// Created by mikag on 4/1/2020.
//

#include "IImporter.h"

IImporter *IImporter::GetImporterForType(RelicType type)
{
    auto importer = registry().find(type);
    if (importer == registry().end()) return nullptr;
    return importer->second();
}

void IImporter::RegisterImporter(RelicType type, IImporterGetter getter)
{
    registry()[type] = getter;
}

void IImporter::ReadBin(void *data, void *dest, size_t &offset, size_t size)
{
    memcpy(dest, (char*) data + offset, size);
    offset += size;
}

std::map<RelicType, IImporterGetter> &IImporter::registry()
{
    static std::map<RelicType, IImporterGetter> registry;
    return registry;
}

