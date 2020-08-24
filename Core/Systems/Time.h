//
// Created by mikag on 18/08/2020.
//

#ifndef RELIC_TIME_H
#define RELIC_TIME_H

#include <Core/ISystem.h>

class Time : public ISystem
{
public:
    void Tick(World &world) override;
    void FrameTick(World &world) override;
    void Init(World &world) override;
    void Shutdown(World &world) override;
};


#endif //RELIC_TIME_H
