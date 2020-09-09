//
// Created by mikag on 9/09/2020.
//

#ifndef RELIC_FPSCAMERASYSTEM_H
#define RELIC_FPSCAMERASYSTEM_H


#include <Core/ISystem.h>

class FPSCameraSystem : public ISystem
{
private:
    static SystemRegistrar registrar;
public:
    void Tick(World &world) override;

    void FrameTick(World &world) override;

    void Init(World &world) override;

    void Shutdown(World &world) override;
};


#endif //RELIC_FPSCAMERASYSTEM_H
