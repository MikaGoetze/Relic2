//
// Created by Mika Goetze on 2019-07-31.
//

#include <Graphics/Window.h>
#include <Graphics/VulkanRenderer.h>
#include <Debugging/Logger.h>
#include "Relic.h"
#include "Time.h"
#include <Libraries/IMGUI/imgui.h>
#include <Libraries/IMGUI/imgui_impl_vulkan.h>
#include <Libraries/IMGUI/imgui_impl_glfw.h>
#include <Graphics/MeshComponent.h>
#include <Core/Components/TransformComponent.h>

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

void Relic::CreateCoreSystems()
{
    renderer = new VulkanRenderer(window, true);
    worlds[0]->RegisterSystem(renderer);
}

void Relic::Initialise()
{
    //Initialise core systems
    resourceManager = new ResourceManager();
    memoryManager = new MemoryManager();

    //Create a default world
    worlds.push_back(new World());

    //Initialise graphics
    glfwInit();
    window = new Window(800, 600, "Relic", true);

    imGuiContext = ImGui::CreateContext();
    ImGui::StyleColorsDark();

    CreateCoreSystems();

    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetWindowPos("Stats", ImVec2(0, 0));
    ImGui::Render();

    DebugInit();
}

void Relic::GameLoop()
{
    while (!window->ShouldClose() && isRunning)
    {
        glfwPollEvents();
        Time::Update();

        bool shouldTick = false;

        if (Time::TickDelta() > tickLength)
        {
            Time::Tick();
            shouldTick = true;
        }

        DrawRenderDebugWidget();

        for (auto world : worlds)
        {
            if(shouldTick) world->Tick();
            world->FrameTick();
        }
    }

    renderer->FinishPendingRenderingOperations();
}

void Relic::DrawRenderDebugWidget()
{
    ImGui::NewFrame();

    ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Frame Info");
    ImGui::PlotLines("", Time::frameTimes, Time::FRAME_COUNT, 0, nullptr, 0.0f, Time::averageTime * 2.0f);

    ImGui::Columns(3);
    ImGui::Separator();
    ImGui::Text("ID");
    ImGui::NextColumn();
    ImGui::Text("CRNT");
    ImGui::NextColumn();
    ImGui::Text("AVG");
    ImGui::Separator();

    ImGui::NextColumn();
    ImGui::Text("FRMT");
    ImGui::NextColumn();
    ImGui::Text("%.5fms", Time::FrameDelta());
    ImGui::NextColumn();
    ImGui::Text("%.5fms", Time::averageTime);
    ImGui::Separator();

    ImGui::NextColumn();
    ImGui::Text("FPS");
    ImGui::NextColumn();
    ImGui::Text("%.0ffps", 1.0f / Time::FrameDelta());
    ImGui::NextColumn();
    ImGui::Text("%.0ffps", 1.0f / Time::averageTime);

    ImGui::End();
}

void Relic::Cleanup()
{
    DebugDestroy();
    ImGui::DestroyContext(imGuiContext);
    delete renderer;
    delete window;
    glfwTerminate();

    delete resourceManager;
    delete memoryManager;
}

///Debug initialisation code, this should be deleted later.
void Relic::DebugInit()
{
    //Load test model
    GUID guid = resourceManager->ImportResource("Resources/Models/cottage.fbx", REL_STRUCTURE_TYPE_MODEL);
    model = resourceManager->GetSimpleResourceData<Model>(guid);
    renderer->PrepareModel(*model);

    auto registry = worlds[0]->Registry();

    for(size_t i = 0; i < model->meshCount; i++)
    {
        auto entity = registry->create();
        registry->emplace<MeshComponent>(entity, &model->meshes[i], model->meshes[i].guid);
        registry->emplace<TransformComponent>(entity);
    }
}

void Relic::DebugDestroy()
{
    renderer->DestroyModel(*model);
}
