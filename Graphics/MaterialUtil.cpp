//
// Created by mikag on 31/08/2020.
//

#include "MaterialUtil.h"

#include <Graphics/Model.h>
#include <ResourceManager/ResourceManager.h>
#include <Core/Relic.h>

Material *MaterialUtil::CreateMaterial(GUID texture)
{
    auto *mat = new Material();
    mat->texture = ResourceManager::GetInstance()->GetSimpleResourceData<Texture>(texture);

    //Register the material with our current renderer in order to init any render state.
    auto * renderer = Relic::Instance()->GetPrimaryWorld()->GetSystem<Renderer>();
    renderer->RegisterMaterial(mat);

    return mat;
}

