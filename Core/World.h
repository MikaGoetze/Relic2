//
// Created by mikag on 22/08/2020.
//

#ifndef RELIC_WORLD_H
#define RELIC_WORLD_H

#include <Libraries/entt/entt.hpp>
#include "ISystem.h"

class World
{
private:
    entt::registry registry;
    std::vector<ISystem*> systems;

public:
    entt::registry * Registry();

    void Tick();
    void FrameTick();
    void RegisterSystem(ISystem* system);
    void RemoveSystem(ISystem* system);

    template <typename T>
    T * GetSystem() const;

    ~World();
};

template<typename T>
T * World::GetSystem() const
{
    T* ptr;
    for(auto * system : systems)
    {
        ptr = dynamic_cast<T*>(system);
        if(ptr != nullptr)
        {
            return ptr;
        }
    }

    return nullptr;
}

#endif //RELIC_WORLD_H
