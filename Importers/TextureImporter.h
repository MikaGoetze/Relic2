//
// Created by mikag on 31/08/2020.
//

#ifndef RELIC_TEXTUREIMPORTER_H
#define RELIC_TEXTUREIMPORTER_H


#include "IImporter.h"

class TextureImporter : public IImporter
{
    static ImporterRegistrar registrar;

    GUID ImportResource(const std::string &filePath) override;

    void *Deserialize(void *data, size_t dataSize) override;

    void *Serialize(void *resource, size_t & totalSize) override;

    static TextureImporter * Instance();

    TextureImporter();
    ~TextureImporter();
};


#endif //RELIC_TEXTUREIMPORTER_H
