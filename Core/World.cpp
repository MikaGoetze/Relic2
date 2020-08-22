//
// Created by mikag on 22/08/2020.
//

#include "World.h"

entt::registry * World::Registry()
{
    return &registry;
}

void World::Tick()
{
    for(auto system : systems)
    {
        if(!system->NeedsTick) continue;
        system->Tick(*this);
    }
}

void World::RegisterSystem(ISystem *system)
{
    systems.push_back(system);
}

void World::RemoveSystem(ISystem *system)
{
    auto iterator = systems.begin();
    while(iterator != systems.end())
    {
        if(*iterator == system)
        {
            systems.erase(iterator);
            break;
        }
    }
}

void World::FrameTick()
{
    for(auto system : systems)
    {
        if(!system->NeedsFrameTick) continue;
        system->FrameTick(*this);
    }
}

