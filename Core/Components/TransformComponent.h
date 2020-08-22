//
// Created by mikag on 22/08/2020.
//

#ifndef RELIC_TRANSFORMCOMPONENT_H
#define RELIC_TRANSFORMCOMPONENT_H

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

struct TransformComponent
{
    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation;
};

#endif //RELIC_TRANSFORMCOMPONENT_H
