//
// Created by mikag on 31/08/2020.
//

#ifndef RELIC_MATERIALUTIL_H
#define RELIC_MATERIALUTIL_H

#include <Importers/ImportUtil.h>

struct Material;

class MaterialUtil
{
public:
    static Material* CreateMaterial(GUID texture);
};

#endif //RELIC_MATERIALUTIL_H
