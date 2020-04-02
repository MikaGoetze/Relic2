//
// Created by mikag on 3/29/2020.
//

#include <Graphics/Model.h>
#include "ModelImporter.h"

GUID ModelImporter::ImportResource(const std::string &filePath)
{

    ofbx::IScene *scene = LoadScene(filePath);

    GUID guid = GetGUID(filePath);

    Model *model = new Model();
    model->meshCount = scene->getMeshCount();
    model->meshes = new Mesh[model->meshCount];
    for (size_t i = 0; i < model->meshCount; i++)
    {
        const ofbx::Geometry *geometry = scene->getMesh(i)->getGeometry();
        model->meshes[i].indexCount = geometry->getIndexCount();

        const int *faceIndices = geometry->getFaceIndices();
        uint32_t *uintFaceIndices = new uint32_t[geometry->getIndexCount()];

        for (size_t j = 0; j < geometry->getIndexCount(); j++)
        {
            uintFaceIndices[j] = static_cast<uint32_t>(faceIndices[j]);
        }

        model->meshes[i].indices = uintFaceIndices;

        model->meshes[i].vertexCount = geometry->getVertexCount();
        const ofbx::Vec3 *verts = geometry->getVertices();
        glm::vec3 *glmVerts = new glm::vec3[geometry->getVertexCount()];

        for (size_t j = 0; j < geometry->getVertexCount(); j++)
        {
            glmVerts[i] = glm::vec3(verts[i].x, verts[i].y, verts[i].z);
        }

        model->meshes[i].vertices = glmVerts;

        model->meshes[i].normalCount = geometry->getIndexCount();
        const ofbx::Vec3 *normals = geometry->getNormals();
        glm::vec3 *glmNormals = new glm::vec3[geometry->getIndexCount()];

        for (size_t j = 0; j < geometry->getIndexCount(); j++)
        {
            glmNormals[i] = glm::vec3(normals[i].x, normals[i].y, normals[i].z);
        }

        model->meshes[i].normals = glmNormals;
    }

    ResourceManager::GetInstance()->SetResourceData(guid, REL_STRUCTURE_TYPE_MODEL, sizeof(Model), model);
    return guid;
}

void *ModelImporter::Deserialize(void *data, size_t dataSize)
{
    return nullptr;
}

void *ModelImporter::Serialize(void *resource, size_t & totalSize)
{
    auto *model = static_cast<Model *>(resource);
    totalSize = sizeof(size_t) + model->meshCount * (3 * sizeof(size_t));

    for(size_t i = 0; i < model->meshCount; i++)
    {
        totalSize += model->meshes[i].vertexCount * sizeof(glm::vec3);
        totalSize += model->meshes[i].normalCount * sizeof(glm::vec3);
        totalSize += model->meshes[i].indexCount * sizeof(uint32_t);
    }

    char * data = new char[totalSize];
    size_t offset = 0;

    /*
     * Layout:
     *
     * Model Count
     * Model[]
     *  VertexCount
     *  Vertices[]
     *  NormalCount
     *  Normals[]
     *  IndexCount
     *  Indices[]
     *
     */

    //Model count
    memcpy(data, &model->meshCount, sizeof(size_t));
    offset += sizeof(size_t);

    //Model array
    for(size_t i = 0; i < model->meshCount; i++)
    {
        memcpy(data + offset, &model->meshes[i].vertexCount, sizeof(size_t));
        offset += sizeof(size_t);

        size_t size = model->meshes[i].vertexCount * sizeof(glm::vec3);
        memcpy(data + offset, model->meshes[i].vertices, size);
        offset += size;

        memcpy(data + offset, &model->meshes[i].normalCount, sizeof(size_t));
        offset += sizeof(size_t);

        size = model->meshes[i].normalCount * sizeof(glm::vec3);
        memcpy(data + offset, model->meshes[i].normals, size);
        offset += size;

        memcpy(data + offset, &model->meshes[i].indexCount, sizeof(size_t));
        offset += sizeof(size_t);

        size = model->meshes[i].normalCount * sizeof(uint32_t);
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
