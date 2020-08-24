//
// Created by mikag on 24/08/2020.
//

#ifndef RELIC_SINGLETONRENDERSTATE_H
#define RELIC_SINGLETONRENDERSTATE_H

#include <Graphics/Window.h>
#include <glm/glm.hpp>

struct SingletonRenderState
{
    Window* window;
    glm::mat4 vpMatrix;
};

#endif //RELIC_SINGLETONRENDERSTATE_H
