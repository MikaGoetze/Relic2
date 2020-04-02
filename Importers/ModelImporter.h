//
// Created by mikag on 3/29/2020.
//

#ifndef RELIC_MODELIMPORTER_H
#define RELIC_MODELIMPORTER_H

#include <Graphics/OpenFBX/FBXUtil.h>
#include <ResourceManager/ResourceManager.h>
#include "ImportUtil.h"
#include "IImporter.h"

class ModelImporter : public IImporter
{
private:
    static ModelImporter* instance;

public:
    static ImporterRegistrar registrar;

    GUID ImportResource(const std::string &filePath) override;

    void *Deserialize(void *data, size_t dataSize) override;

    void *Serialize(void *resource, size_t & totalSize) override;

    static ModelImporter * GetInstance();

    ModelImporter();
    ~ModelImporter();
};

#endif //RELIC_MODELIMPORTER_H
