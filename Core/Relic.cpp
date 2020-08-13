//
// Created by Mika Goetze on 2019-07-31.
//

#include <Graphics/Window.h>
#include <Graphics/VulkanRenderer.h>
#include <Debugging/Logger.h>
#include "Relic.h"

Relic::Relic()
{
    window = nullptr;
    isRunning = false;
    renderer = nullptr;
    resourceManager = nullptr;
    memoryManager = nullptr;
}

Relic::~Relic()
= default;

void Relic::Start()
{
    isRunning = true;
    Initialise();
    GameLoop();
    Cleanup();
}

void Relic::Shutdown()
{
    isRunning = false;
}

void Relic::Initialise()
{
    //Initialise core systems
    resourceManager = new ResourceManager();
    memoryManager = new MemoryManager();

    //Initialise graphics
    glfwInit();
    window = new Window(800, 600, "Relic", true);
    renderer = new VulkanRenderer(window, true);
}

void Relic::GameLoop()
{
    while (!window->ShouldClose() && isRunning)
    {
        glfwPollEvents();

        DrawFrame();
    }

    renderer->FinishPendingRenderingOperations();
}

void Relic::Cleanup()
{
    delete renderer;
    delete window;
    glfwTerminate();

    delete resourceManager;
    delete memoryManager;
}

void Relic::DrawFrame()
{
    renderer->Render();
}
