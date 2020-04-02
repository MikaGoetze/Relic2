//
// Created by mikag on 4/1/2020.
//

#include "IImporter.h"

std::map<RelicType, IImporterGetter> IImporter::registry;

IImporter *IImporter::GetImporterForType(RelicType type)
{
    auto importer = registry.find(type);
    if(importer == registry.end()) return nullptr;
    return importer->second();
}

void IImporter::RegisterImporter(RelicType type, IImporterGetter getter)
{
    registry[type] = getter;
}
