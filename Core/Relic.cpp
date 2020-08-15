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

    //Load test model
//    GUID guid = resourceManager->ImportResource("cube.fbx", REL_STRUCTURE_TYPE_MODEL);
//    Model* model = resourceManager->GetSimpleResourceData<Model>(guid);

    Mesh mesh = {};
    mesh.vertexCount = 3;
    mesh.vertices = new Vertex[3]{
            {
                    glm::vec3(-1.0f, 1.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::vec2(0.0f, 0.0f)
            },
            {
                    glm::vec3(1.0f, 1.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::vec2(0.0f, 0.0f)
            },
            {
                    glm::vec3(0.0f, -1.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::vec2(0.0f, 0.0f)
            },
    };

    mesh.indexCount = 3;
    mesh.indices = new uint32_t[3]
    {
            2,
            1,
            0
    };

    renderer = new VulkanRenderer(window, true, &mesh);
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
