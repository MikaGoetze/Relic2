#include <iostream>
#include <ResourceManager/Compression/CompressionManager.h>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <Graphics/Window.h>


int main()
{
    glfwInit();

    auto *window = new Window(800, 600, "Relic", true);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    while (!window->ShouldClose()) glfwPollEvents();

    delete window;

    glfwTerminate();

    return 0;
}