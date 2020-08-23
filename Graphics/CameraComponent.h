//
// Created by mikag on 23/08/2020.
//

#ifndef RELIC_CAMERACOMPONENT_H
#define RELIC_CAMERACOMPONENT_H

#include <glm/vec3.hpp>

struct CameraComponent
{
    float fov;
    float nearPlane;
    float farPlane;
    bool isActive;
};

#endif //RELIC_CAMERACOMPONENT_H
