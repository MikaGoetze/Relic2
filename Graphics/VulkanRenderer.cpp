//
// Created by Mika Goetze on 2019-07-31.
//

#include <GLFW/glfw3.h>
#include <Debugging/Logger.h>
#include <cstring>
#include <vector>
#include "VulkanRenderer.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

void VulkanRenderer::CreateInstance(bool enableValidationLayers)
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
        Logger::Log(2, "[VulkanRenderer] Found support for: ", glfwExtensions[i]);
    }

    instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
    instanceCreateInfo.enabledLayerCount = 0;

    std::vector<const char *> layers = {
            "VK_LAYER_KHRONOS_validation"
    };
    if (enableValidationLayers) EnableValidationLayers(instanceCreateInfo, layers);

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS)
    {
        Logger::Log(1, "[VulkanRenderer] Failed to create instance...");
    }
    Logger::Log(1, "[VulkanRenderer] Created instance successfully.");
}

VulkanRenderer::VulkanRenderer(bool enableValidationLayers)
{
    instance = VkInstance{};
    supportedExtensionCount = 0;
    supportedExtensions = nullptr;
    supportedValidationLayers = nullptr;
    supportedValidationLayersCount = 0;
    enabledValidationLayers = new std::vector<const char *>();

    CreateInstance(enableValidationLayers);
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

    for (uint32_t i = 0; i < supportedExtensionCount; i++)
    {
        if (strcmp(extensionName, supportedExtensions[i].extensionName) == 0) return true;
    }
    return false;
}

VulkanRenderer::~VulkanRenderer()
{
    delete supportedExtensions;
    delete supportedValidationLayers;
    delete enabledValidationLayers;
    vkDestroyInstance(instance, nullptr);
}

bool VulkanRenderer::ValidationLayerSupported(const char *validationLayerName)
{
    if (supportedValidationLayers == nullptr)
    {
        vkEnumerateInstanceLayerProperties(&supportedValidationLayersCount, nullptr);
        supportedValidationLayers = new VkLayerProperties[supportedValidationLayersCount];
        vkEnumerateInstanceLayerProperties(&supportedValidationLayersCount, supportedValidationLayers);
    }

    for (uint32_t i = 0; i < supportedValidationLayersCount; i++)
    {
        if (strcmp(supportedValidationLayers[i].layerName, validationLayerName) == 0) return true;
    }

    return false;
}

void VulkanRenderer::EnableValidationLayers(VkInstanceCreateInfo &createInfo, const std::vector<const char *> &layers)
{
    for (auto &validationLayer : layers)
    {
        if (!ValidationLayerSupported(validationLayer))
        {
            Logger::Log(3, "[VulkanRenderer] Validation layer ", validationLayer, " is not supported.");
            continue;
        }
        enabledValidationLayers->push_back(validationLayer);
        Logger::Log(2, "[VulkanRenderer] Found support for layer: ", validationLayer);
    }

    createInfo.enabledLayerCount = enabledValidationLayers->size();
    createInfo.ppEnabledLayerNames = enabledValidationLayers->data();
}

#pragma clang diagnostic pop
