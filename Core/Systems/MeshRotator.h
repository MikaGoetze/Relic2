//
// Created by mikag on 23/08/2020.
//

#ifndef RELIC_MESHROTATOR_H
#define RELIC_MESHROTATOR_H


#include <Core/ISystem.h>

class MeshRotator : public ISystem
{
    void Tick(World &world) override;
    void FrameTick(World &world) override;

public:
    MeshRotator();
};


#endif //RELIC_MESHROTATOR_H
