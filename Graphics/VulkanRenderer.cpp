//
// Created by Mika Goetze on 2019-07-31.
//

#include <GLFW/glfw3.h>
#include <Debugging/Logger.h>
#include <cstring>
#include "VulkanRenderer.h"

void VulkanRenderer::Initialise()
{
    CreateInstance();
}

void VulkanRenderer::Cleanup()
{

}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

void VulkanRenderer::CreateInstance()
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.apiVersion = VK_API_VERSION_1_0;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.pApplicationName = "Test";
    applicationInfo.pEngineName = "Relic";
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    //Confirm that all extensions are available.
    for (uint32_t i = 0; i < glfwExtensionCount; i++)
    {
        if (!ExtensionSupported(glfwExtensions[i]))
        {
            Logger::Log(2, "[VulkanRenderer] Extension not available:", glfwExtensions[i]);
            return;
        }
    }

    instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
    instanceCreateInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS)
    {
        Logger::Log(1, "[VulkanRenderer] Failed to create instance...");
    }
    Logger::Log(1, "[VulkanRenderer] Created instance successfully.");
}

VulkanRenderer::VulkanRenderer()
{
    instance = VkInstance{};
    supportedExtensionCount = 0;
    supportedExtensions = nullptr;
}

bool VulkanRenderer::ExtensionSupported(const char *extensionName)
{
    if (supportedExtensions == nullptr)
    {
        //Get extension count first
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
        supportedExtensions = new VkExtensionProperties[supportedExtensionCount];
        //Then fill the array
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions);
    }

    Logger::Log(2, "[VulkanRenderer] Checking support for: ", extensionName);
    for (uint32_t i = 0; i < supportedExtensionCount; i++)
    {
        if (strcmp(extensionName, supportedExtensions[i].extensionName) == 0) return true;
    }
    return false;
}

#pragma clang diagnostic pop
