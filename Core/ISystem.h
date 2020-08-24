//
// Created by mikag on 22/08/2020.
//

#ifndef RELIC_ISYSTEM_H
#define RELIC_ISYSTEM_H

#include <Libraries/entt/entt.hpp>
#include <map>
#include <Debugging/Logger.h>

class World;

class ISystem
{
public:
    virtual void Tick(World& world) = 0;
    virtual void FrameTick(World &world) = 0;

    virtual void Init(World &world) = 0;
    virtual void Shutdown() = 0;

    bool NeedsTick = true;
    bool NeedsFrameTick = false;

    static std::vector<ISystem*>& SystemRegistry()
    {
        static std::vector<ISystem *> registrar;
        return registrar;
    }
};

struct SystemRegistrar
{
    explicit SystemRegistrar(ISystem* system)
    {
        ISystem::SystemRegistry().push_back(system);
        Logger::Log("Registering system '%s'", typeid(*system).name());
    }
};


#endif //RELIC_ISYSTEM_H
