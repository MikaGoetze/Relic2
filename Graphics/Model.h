//
// Created by mikag on 3/29/2020.
//

#ifndef RELIC_MODEL_H
#define RELIC_MODEL_H

#include <cstdint>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <Importers/ImportUtil.h>
#include <Core/RelicStruct.h>
#include <memory>

typedef struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoordinate;
} Vertex;

struct Mesh : RelicStruct
{
   uint32_t sType = REL_STRUCTURE_TYPE_MESH;

   Vertex * vertices = nullptr;
   size_t vertexCount;

   uint32_t * indices = nullptr;
   size_t indexCount;

   void* renderData = nullptr;

    GUID guid;
} ;

struct Material : RelicStruct
{
    uint32_t sType = REL_STRUCTURE_TYPE_MATERIAL;
    std::string filename;
};

struct Model : RelicStruct
{
    RelicType sType = REL_STRUCTURE_TYPE_MODEL;
    size_t meshCount;
    Mesh * meshes = nullptr;
    size_t materialCount;
    Material* materials = nullptr;

    GUID guid;
};


#endif //RELIC_MODEL_H
