//
// Created by Mika Goetze on 2019-07-31.
//

#ifndef RELIC_RELIC_H
#define RELIC_RELIC_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <Graphics/Window.h>
#include <Graphics/VulkanRenderer.h>

class Relic
{
public:
    Relic();

    ~Relic();

    void Start();

    void Shutdown();

private:
    void Initialise();

    void GameLoop();

    void Cleanup();

    Window *window;
    Renderer *renderer;
    bool isRunning;
};


#endif //RELIC_RELIC_H
