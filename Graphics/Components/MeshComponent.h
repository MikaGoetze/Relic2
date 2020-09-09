//
// Created by mikag on 22/08/2020.
//

#ifndef RELIC_MESHCOMPONENT_H
#define RELIC_MESHCOMPONENT_H

#include <Importers/ImportUtil.h>
#include "Graphics/Model.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

struct MeshComponent
{
    Mesh *mesh;
    Material* material;
    GUID guid;
};

#endif //RELIC_MESHCOMPONENT_H
