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
    GUID guid = resourceManager->ImportResource("Resources/Models/Handgun.fbx", REL_STRUCTURE_TYPE_MODEL);
    Model* model = resourceManager->GetSimpleResourceData<Model>(guid);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    renderer = new VulkanRenderer(window, model, true);

    //Setup initial window pos

    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetWindowPos("Stats", ImVec2(0, 0));
    ImGui::Render();
}

void Relic::GameLoop()
{
    while (!window->ShouldClose() && isRunning)
    {
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_AlwaysAutoResize);



        ImGui::End();

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
