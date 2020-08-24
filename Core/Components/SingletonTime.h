//
// Created by mikag on 24/08/2020.
//

#ifndef RELIC_SINGLETONTIME_H
#define RELIC_SINGLETONTIME_H

#include <Core/IComponent.h>

class SingletonTime : public IComponent
{
public:
    float currentTime;
    float lastFrameTime;
    float lastTickTime;

    float TickDelta()
    {
        return currentTime - lastTickTime;
    }

    float FrameDelta()
    {
        return currentTime - lastFrameTime;
    }
};

#endif //RELIC_SINGLETONTIME_H
