//
// Created by mikag on 22/08/2020.
//

#ifndef RELIC_ISYSTEM_H
#define RELIC_ISYSTEM_H

#include <Libraries/entt/entt.hpp>

class World;

class ISystem
{
public:
    virtual void Tick(World& world) = 0;
    virtual void FrameTick(World &world) = 0;
    bool NeedsTick = true;
    bool NeedsFrameTick = false;
};


#endif //RELIC_ISYSTEM_H
