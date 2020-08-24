//
// Created by Mika Goetze on 2019-07-31.
//

#include <Graphics/Window.h>
#include <Graphics/VulkanRenderer.h>
#include "Relic.h"
#include "Time.h"
#include <Libraries/IMGUI/imgui_impl_vulkan.h>
#include <Libraries/IMGUI/imgui_impl_glfw.h>
#include <Graphics/MeshComponent.h>
#include <Graphics/CameraComponent.h>
#include <Core/Systems/MeshRotator.h>


Relic::Relic()
{
    window = nullptr;
    isRunning = false;
    renderer = nullptr;
    resourceManager = nullptr;
    memoryManager = nullptr;

    instance = this;
}

Relic::~Relic()
{
    instance = nullptr;
}

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

void Relic::CreateSystems()
{
    Logger::Log("Creating %i registered systems", ISystem::SystemRegistry().size());
    for(auto system : ISystem::SystemRegistry())
    {
        worlds[0]->RegisterSystem(system);
        system->Init(*worlds[0]);
    }
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

    CreateSystems();
    CreateDefaultWorldObjects();

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

        bool shouldTick = false;

        if (Time::TickDelta() > tickLength)
        {
            shouldTick = true;
        }

        DrawRenderDebugWidget();

        for (auto world : worlds)
        {
            if(shouldTick) world->Tick();
            world->FrameTick();
        }

        if(shouldTick) Time::Tick();
        Time::FrameTick();
    }
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

    for(auto world : worlds)
    {
        delete world;
    }

    ImGui::DestroyContext(imGuiContext);
    DebugDestroy();

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

    auto registry = worlds[0]->Registry();

    for(size_t i = 0; i < model->meshCount; i++)
    {
        auto entity = registry->create();
        registry->emplace<MeshComponent>(entity, &model->meshes[i], model->meshes[i].guid);
        registry->emplace<TransformComponent>(entity, glm::zero<glm::vec3>(), glm::one<glm::vec3>(), glm::quat(glm::vec3(-glm::radians(90.0f), 0, 0)));
    }
}

void Relic::DebugDestroy()
{
}

void Relic::CreateDefaultWorldObjects()
{
    auto registry = worlds[0]->Registry();
    auto camera = registry->create();

    registry->emplace<CameraComponent>(camera, 45.0f, 0.5f, 200.0f, true);
    registry->emplace<TransformComponent>(camera, glm::vec3(0, 30, -60), glm::one<glm::vec3>(), glm::vec3(-glm::radians(-30.0f), 0, 0));
}

const Relic *Relic::Instance()
{
    return static_cast<const Relic*>(instance);
}

Window *Relic::GetActiveWindow() const
{
    return window;
}

Relic* Relic::instance = nullptr;
