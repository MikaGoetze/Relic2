//
// Created by Mika Goetze on 2019-07-31.
//

#include <Graphics/Window.h>
#include <Graphics/Systems/VulkanRenderer.h>
#include "Relic.h"
#include "Core/Systems/Time.h"
#include <Libraries/IMGUI/imgui_impl_vulkan.h>
#include <Libraries/IMGUI/imgui_impl_glfw.h>
#include <Graphics/Components/MeshComponent.h>
#include <Graphics/Components/CameraComponent.h>
#include <Core/Systems/MeshRotator.h>
#include <Core/Components/SingletonTime.h>
#include <Core/Components/SingletonFrameStats.h>


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
    //Create our core systems in order to guarantee correct execution order
    Time* time = new Time();
    worlds[0]->RegisterSystem(time);
    time->Init(*worlds[0]);

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

        DrawRenderDebugWidget();

        for (auto world : worlds)
        {
            world->FrameTick();

            if (world->Registry()->ctx<SingletonTime*>()->TickDelta() > tickLength)
            {
                world->Tick();
            }
        }
    }
}

void Relic::DrawRenderDebugWidget()
{
    //we only draw stats for our default world.
    ImGui::NewFrame();

    ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Frame Info");

    SingletonTime* time = worlds[0]->Registry()->ctx<SingletonTime*>();
    SingletonFrameStats* frameStats = worlds[0]->Registry()->ctx<SingletonFrameStats*>();

    ImGui::PlotLines("", frameStats->frameTimes, SingletonFrameStats::FRAME_COUNT, 0, nullptr, 0.0f, frameStats->averageFrameTime * 2.0f);

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
    ImGui::Text("%.5fms", time->FrameDelta());
    ImGui::NextColumn();
    ImGui::Text("%.5fms", frameStats->averageFrameTime);
    ImGui::Separator();

    ImGui::NextColumn();
    ImGui::Text("FPS");
    ImGui::NextColumn();
    ImGui::Text("%.0ffps", 1.0f / time->FrameDelta());
    ImGui::NextColumn();
    ImGui::Text("%.0ffps", 1.0f / frameStats->averageFrameTime);

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

        auto test = TransformComponent{glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::quat()};
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
