//
// Created by mikag on 22/08/2020.
//

#ifndef RELIC_MESHCOMPONENT_H
#define RELIC_MESHCOMPONENT_H

#include <Importers/ImportUtil.h>

struct MeshComponent
{
    Mesh *mesh;
    GUID guid;

//    MeshComponent(const MeshComponent &other)
//    {
//        mesh = other.mesh;
//        guid = other.guid;
//    }
//
//    MeshComponent(const MeshComponent &&other) noexcept
//    {
//        mesh = other.mesh;
//        guid = other.guid;
//    }
//
//    MeshComponent &operator=(const MeshComponent &other) noexcept
//    {
//        if (this != &other)
//        {
//            mesh = other.mesh;
//            guid = other.guid;
//        }
//        return *this;
//    }
//
//    MeshComponent(Mesh *m, GUID g)
//    {
//        mesh = m;
//        guid = g;
//    }
//
//    MeshComponent()
//    {
//        mesh = nullptr;
//        guid = 0;
//    }
};

#endif //RELIC_MESHCOMPONENT_H
