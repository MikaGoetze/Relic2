//
// Created by mikag on 9/09/2020.
//

#ifndef RELIC_FPSCAMERACOMPONENT_H
#define RELIC_FPSCAMERACOMPONENT_H



struct FPSCameraComponent
{
    float sensitivity = 0.05f;
    float yaw = 0;
    float pitch = 90;
    float lastMouseX = 0;
    float lastMouseY = 0;
    bool initialised;
    float moveSpeed = 10.0f;
};


#endif //RELIC_FPSCAMERACOMPONENT_H
