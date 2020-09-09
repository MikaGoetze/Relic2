//
// Created by mikag on 9/09/2020.
//

#ifndef RELIC_INPUT_H
#define RELIC_INPUT_H


#include <Core/ISystem.h>

class GLFWwindow;

class Input : public ISystem
{
public:
    void Tick(World &world) override;

    void FrameTick(World &world) override;

    void Init(World &world) override;

    void Shutdown(World &world) override;

private:
    static SystemRegistrar registrar;
    static void OnMouseMoved(GLFWwindow* window, double x, double y);
};


#endif //RELIC_INPUT_H
