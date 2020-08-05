//
// Created by mikag on 3/29/2020.
//

#include <Graphics/Model.h>
#include "ModelImporter.h"

GUID ModelImporter::ImportResource(const std::string &filePath)
{
    ofbx::IScene *scene = LoadScene(filePath);

    GUID guid = GetGUID(filePath);

    auto *model = new Model();
    model->meshCount = scene->getMeshCount();
    model->meshes = new Mesh[model->meshCount];
    for (size_t i = 0; i < model->meshCount; i++)
    {
        const ofbx::Geometry *geometry = scene->getMesh(i)->getGeometry();
        model->meshes[i].indexCount = geometry->getIndexCount();

        const int *faceIndices = geometry->getFaceIndices();
        auto *uintFaceIndices = new uint32_t[geometry->getIndexCount()];

        for (size_t j = 0; j < geometry->getIndexCount(); j++)
        {
            uintFaceIndices[j] = static_cast<uint32_t>(faceIndices[j]);
        }

        model->meshes[i].indices = uintFaceIndices;

        model->meshes[i].vertexCount = geometry->getVertexCount();
        const ofbx::Vec3 *verts = geometry->getVertices();
        const ofbx::Vec3 *normals = geometry->getNormals();

        auto *relVerts = new Vertex[geometry->getVertexCount()];

        for (size_t j = 0; j < geometry->getVertexCount(); j++)
        {
            Vertex vertex = {};
            vertex.position = glm::vec3(verts[i].x, verts[i].y, verts[i].z);
            vertex.normal = glm::vec3(normals[i].x, normals[i].y, normals[i].z);
            //TODO: Init tex coords.

            relVerts[i] = vertex;
        }

        model->meshes[i].vertices = relVerts;
    }

    ResourceManager::GetInstance()->SetResourceData(guid, REL_STRUCTURE_TYPE_MODEL, sizeof(Model), model);
    return guid;
}

void *ModelImporter::Deserialize(void *data, size_t dataSize)
{
    /*
     * Layout:
     *
     * Mesh Count
     * Mesh[]
     *  VertexCount
     *  Vertices[]
     *  IndexCount
     *  Indices[]
     *
     */

    size_t offset = 0;
    auto *model = new Model;

    size_t meshCount;
    ReadBin(data, &meshCount, offset, sizeof(size_t));

    //TODO: This should all use custom allocators instead of new most likely...
    model->meshCount = meshCount;
    model->meshes = new Mesh[meshCount];

    for(int i = 0; i < meshCount; i++)
    {
        ReadBin(data, &model->meshes[i].vertexCount, offset, sizeof(size_t));
        model->meshes[i].vertices = new Vertex[model->meshes[i].vertexCount];

        for(int j = 0; j < model->meshes[i].vertexCount; j++)
        {
            ReadBin(data, &model->meshes[i].vertices[j], offset, sizeof(Vertex));
        }

        ReadBin(data, &model->meshes[i].indexCount, offset, sizeof(size_t));
        model->meshes[i].indices = new uint32_t[model->meshes[i].indexCount];

        for(int j = 0; j < model->meshes[i].indexCount; j++)
        {
            ReadBin(data, &model->meshes[i].indices[j], offset, sizeof(uint32_t));
        }
    }

    return model;
}

void *ModelImporter::Serialize(void *resource, size_t &totalSize)
{
    auto *model = static_cast<Model *>(resource);

    totalSize = sizeof(size_t) + (sizeof(size_t) * 2) * model->meshCount;

    for (size_t i = 0; i < model->meshCount; i++)
    {
        totalSize += model->meshes[i].vertexCount * sizeof(Vertex);
        totalSize += model->meshes[i].indexCount * sizeof(uint32_t);
    }

    char *data = new char[totalSize];
    size_t offset = 0;

    /*
     * Layout:
     *
     * Mesh Count
     * Mesh[]
     *  VertexCount
     *  Vertices[]
     *  IndexCount
     *  Indices[]
     *
     */

    //Model count
    memcpy(data, &model->meshCount, sizeof(size_t));
    offset += sizeof(size_t);

    //Model array
    for (size_t i = 0; i < model->meshCount; i++)
    {
        memcpy(data + offset, &model->meshes[i].vertexCount, sizeof(size_t));
        offset += sizeof(size_t);

        size_t size = model->meshes[i].vertexCount * sizeof(Vertex);
        memcpy(data + offset, model->meshes[i].vertices, size);
        offset += size;

        memcpy(data + offset, &model->meshes[i].indexCount, sizeof(size_t));
        offset += sizeof(size_t);

        size = model->meshes[i].indexCount * sizeof(uint32_t);
        memcpy(data + offset, model->meshes[i].indices, size);
        offset += size;
    }

    return data;
}

ModelImporter::ModelImporter()
{
    instance = this;
}

ModelImporter::~ModelImporter()
{
    instance = nullptr;
}

ModelImporter *ModelImporter::instance = nullptr;

ModelImporter *ModelImporter::GetInstance()
{
    return instance;
}

ImporterRegistrar ModelImporter::registrar(REL_STRUCTURE_TYPE_MODEL, reinterpret_cast<IImporterGetter>(&GetInstance));
