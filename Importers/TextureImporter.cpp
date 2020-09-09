//
// Created by mikag on 31/08/2020.
//

#include "TextureImporter.h"
#include "ModelImporter.h"
#define STB_IMAGE_IMPLEMENTATION
#include <Graphics/stb_image.h>
#include <memory>
#include <Graphics/Texture.h>
#include <cstring>


GUID TextureImporter::ImportResource(const std::string &filePath)
{
    int width, height, numComponents;
    std::unique_ptr<unsigned char, void(*)(void*)> stbi_data(stbi_load(filePath.c_str(), &width, &height, &numComponents, 4), stbi_image_free);

    if(width <= 0 || height <= 0 || numComponents <= 0) return GUID_INVALID;

    auto * texture = new Texture();
    GUID guid = GetGUID(filePath);

    texture->width = width;
    texture->height = height;
    texture->numComponents = numComponents;
    texture->dataSize = width * height * numComponents;

    texture->data = new unsigned char[texture->dataSize];
    std::memcpy(texture->data, stbi_data.get(), texture->dataSize);

    ResourceManager::GetInstance()->SetResourceData(guid, REL_STRUCTURE_TYPE_TEXTURE, sizeof(Texture), texture);
    return guid;
}

void *TextureImporter::Deserialize(void *data, size_t dataSize)
{
    return nullptr;
}

void *TextureImporter::Serialize(void *resource, size_t &totalSize)
{
    return nullptr;
}

TextureImporter *TextureImporter::Instance()
{
    static TextureImporter instance;
    return &instance;
}

TextureImporter::TextureImporter()
{
}

TextureImporter::~TextureImporter()
{

}

ImporterRegistrar TextureImporter::registrar(REL_STRUCTURE_TYPE_TEXTURE, reinterpret_cast<IImporterGetter>(&Instance));