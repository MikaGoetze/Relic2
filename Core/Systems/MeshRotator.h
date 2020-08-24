//
// Created by mikag on 23/08/2020.
//

#ifndef RELIC_MESHROTATOR_H
#define RELIC_MESHROTATOR_H


#include <Core/ISystem.h>

class MeshRotator : public ISystem
{
private:
    void Tick(World &world) override;
    void FrameTick(World &world) override;
    static SystemRegistrar registrar;

public:
    MeshRotator();

    void Init(World &world) override;

    void Shutdown(World &world) override;
};


#endif //RELIC_MESHROTATOR_H
