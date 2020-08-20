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
    GUID guid = resourceManager->ImportResource("Resources/Models/cottage.fbx", REL_STRUCTURE_TYPE_MODEL);
    Model* model = resourceManager->GetSimpleResourceData<Model>(guid);

    imGuiContext = ImGui::CreateContext();
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
        Time::Update();

        // Start the ImGui frame

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
        ImGui::Text("%.5fms", Time::DeltaTime());
        ImGui::NextColumn();
        ImGui::Text("%.5fms", Time::averageTime);
        ImGui::Separator();

        ImGui::NextColumn();
        ImGui::Text("FPS");
        ImGui::NextColumn();
        ImGui::Text("%.0ffps", 1.0f / Time::DeltaTime());
        ImGui::NextColumn();
        ImGui::Text("%.0ffps", 1.0f / Time::averageTime);

        ImGui::End();

        DrawFrame();
    }

    renderer->FinishPendingRenderingOperations();
}

void Relic::Cleanup()
{
    ImGui::DestroyContext(imGuiContext);
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
