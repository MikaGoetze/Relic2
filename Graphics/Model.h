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

typedef struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoordinate;
} Vertex;

typedef struct Mesh : RelicStruct
{
    const uint32_t sType = REL_STRUCTURE_TYPE_MESH;

   Vertex * vertices;
   size_t vertexCount;

   uint32_t * indices;
   size_t indexCount;

    GUID guid;
} Mesh;

typedef struct Model : RelicStruct
{
    const RelicType sType = REL_STRUCTURE_TYPE_MODEL;
    size_t meshCount;
    Mesh * meshes;
    GUID guid;
} Model;


#endif //RELIC_MODEL_H