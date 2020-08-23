//
// Created by Mika Goetze on 2019-07-31.
//

#ifndef RELIC_RELIC_H
#define RELIC_RELIC_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <Graphics/Window.h>
#include <Graphics/VulkanRenderer.h>
#include <ResourceManager/ResourceManager.h>
#include <MemoryManager/MemoryManager.h>
#include "World.h"

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

    void DebugInit();
    void DebugDestroy();

    Window *window;
    Renderer *renderer;

    std::vector<World*> worlds;

    ResourceManager *resourceManager;
    MemoryManager *memoryManager;
    bool isRunning;

    float tickLength = 1.0f / 30.0f;

    ImGuiContext *imGuiContext;

    void DrawRenderDebugWidget();

    void CreateCoreSystems();

    //TEMP
    Model *model;

    void CreateDefaultWorldObjects();
};


#endif //RELIC_RELIC_H
