//
// Created by Mika Goetze on 2019-07-31.
//

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <Debugging/Logger.h>
#include <cstring>
#include <string>
#include <map>
#include <set>
#include "VulkanRenderer.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

bool VulkanRenderer::CreateInstance()
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

    std::vector<const char *> *extensions = GetRequiredExtensions();

    instanceCreateInfo.enabledExtensionCount = extensions->size();
    instanceCreateInfo.ppEnabledExtensionNames = extensions->data();

    if (validationLayersEnabled) EnableValidationLayers(instanceCreateInfo, layers);

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS)
    {
        Logger::Log(1, "[VulkanRenderer] Failed to create instance...");
        return false;
    }
    Logger::Log(1, "[VulkanRenderer] Created instance successfully.");

    delete extensions;
    if (instanceCreateInfo.pNext != nullptr) delete (VkDebugUtilsMessengerCreateInfoEXT *) instanceCreateInfo.pNext;
    return true;
}

VulkanRenderer::VulkanRenderer(Window *window, bool enableValidation) : Renderer(window)
{
    instance = VkInstance{};
    supportedExtensionCount = 0;
    supportedExtensions = nullptr;
    supportedValidationLayers = nullptr;
    supportedValidationLayersCount = 0;
    enabledValidationLayers = new std::vector<const char *>();
    validationLayersEnabled = enableValidation;
    debugMessenger = {};

    //Try to create an instance, otherwise exit.
    if (!CreateInstance()) exit(0);
    InitialiseDebugMessenger();

    CreateSurface();

    if (!SelectPhysicalDevice())
    {
        Logger::Log(1, "[VulkanRenderer] Failed to pick physical device.");
    } else
    {
        Logger::Log(1, "[VulkanRenderer] Selected physical device.");
    }

    CreateLogicalDevice();

    CreateSwapChain();
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
    DestroyDebugMessenger(instance, debugMessenger, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
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

    auto *debugCreateInfo = new VkDebugUtilsMessengerCreateInfoEXT();
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) debugCreateInfo;
}

std::vector<const char *> *VulkanRenderer::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> desiredExtensions;
    for (int i = 0; i < glfwExtensionCount; i++) desiredExtensions.push_back(glfwExtensions[i]);

    if (validationLayersEnabled) desiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    auto *extensions = new std::vector<const char *>();

    //Confirm that all extensions are available.
    for (auto &extension : desiredExtensions)
    {
        if (!ExtensionSupported(extension))
        {
            Logger::Log(2, "[VulkanRenderer] Extension not available:", extension);
            continue;
        }
        extensions->push_back(extension);
        Logger::Log(2, "[VulkanRenderer] Found support for: ", extension);
    }

    return extensions;
}

VkBool32 VulkanRenderer::ValidationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                                            const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData)
{
    Logger::Log(2, "[ValidationMessage] ", callbackData->pMessage);

    return VK_FALSE;
}

void VulkanRenderer::InitialiseDebugMessenger()
{
    if (!validationLayersEnabled) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};

    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugMessenger(&instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        Logger::Log(1, "[VulkanRenderer] Failed to create debug messenger");
        return;
    }
}

VkResult
VulkanRenderer::CreateDebugMessenger(VkInstance *instance, const VkDebugUtilsMessengerCreateInfoEXT *createInfo, const
VkAllocationCallbacks *allocatorCallback, VkDebugUtilsMessengerEXT *messenger)
{
    auto function = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(*instance,
                                                                               "vkCreateDebugUtilsMessengerEXT");
    if (function != nullptr)
    {
        return function(*instance, createInfo, allocatorCallback, messenger);
    } else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanRenderer::DestroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                           VkAllocationCallbacks *callback)
{
    auto function = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                                "vkDestroyDebugUtilsMessengerEXT");
    if (function != nullptr)
    {
        Logger::Log(1, "[VulkanRenderer] Removed messenger.");
        function(instance, messenger, callback);
    }
}

void VulkanRenderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    createInfo.pUserData = nullptr;
    createInfo.pfnUserCallback = ValidationCallback;
}

bool VulkanRenderer::SelectPhysicalDevice()
{
    uint32_t numDevices;
    vkEnumeratePhysicalDevices(instance, &numDevices, nullptr);

    if (numDevices == 0)
    {
        Logger::Log(1, "[VulkanRenderer] Found no physical devices...");
        return false;
    }

    auto *devices = new VkPhysicalDevice[numDevices];
    vkEnumeratePhysicalDevices(instance, &numDevices, devices);

    std::multimap<int, VkPhysicalDevice> scoredDevices;

    for (uint32_t i = 0; i < numDevices; i++)
    {
        VkPhysicalDevice &device = devices[i];

        int score = ScorePhysicalDevice(device);
        if (score != 0) scoredDevices.insert(std::make_pair(score, device));
    }

    if (scoredDevices.empty()) return false;
    physicalDevice = scoredDevices.rbegin()->second;

    return true;
}

int VulkanRenderer::ScorePhysicalDevice(VkPhysicalDevice &device)
{
    int score = 0;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    //Highly score dGPUs.
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;

    score += deviceProperties.limits.maxImageDimension2D;

    //We're going to ignore this for now.
//    if(!deviceFeatures.geometryShader) score = 0;

    //If we don't support graphics, this device is not appropriate
    QueueFamilyIndices indices = FindQueueFamily(device);

    bool areExtensionsSupported = CheckDeviceExtensionSupport(device);
    bool isSwapchainSupported;
    if (areExtensionsSupported)
    {
        SwapChainSupportDetails details = QuerySwapChainSupport(device);
        isSwapchainSupported = !details.presentModes.empty() && !details.formats.empty();
    }

    if (!indices.IsComplete() || !areExtensionsSupported || !isSwapchainSupported) score = 0;

    return score;
}

VkSurfaceFormatKHR VulkanRenderer::SelectSwapChainSurfaceFormat(std::vector<VkSurfaceFormatKHR> &formats)
{
    for (auto &format : formats)
    {
        if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_R8G8B8A8_UNORM)
            return format;
    }

    return formats[0];
}

VkPresentModeKHR VulkanRenderer::SelectSwapChainPresentMode(std::vector<VkPresentModeKHR> modes)
{
    for (auto &mode : modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::SelectSwapChainExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    } else
    {
        VkExtent2D actualExtent = {static_cast<uint32_t>(window->GetWindowWidth()),
                                   static_cast<uint32_t>(window->GetWindowHeight())};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width,
                                                                                  actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height,
                                                                                    actualExtent.height));

        return actualExtent;
    }
}

QueueFamilyIndices VulkanRenderer::FindQueueFamily(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    auto *queueFamilies = new VkQueueFamilyProperties[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentationSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);

        if (presentationSupport)
        {
            indices.presentationFamily = i;
        }

        if (indices.IsComplete()) break;
    }

    delete[] queueFamilies;
    return indices;
}

void VulkanRenderer::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamily(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
            indices.presentationFamily.value(),
            indices.graphicsFamily.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
    if (enabledValidationLayers)
    {
        deviceCreateInfo.enabledLayerCount = enabledValidationLayers->size();
        deviceCreateInfo.ppEnabledLayerNames = enabledValidationLayers->data();
    } else
    {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("[VulkanRenderer] Failed to create logical device.");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentationFamily.value(), 0, &presentationQueue);

    Logger::Log(1, "[VulkanRenderer] Created logical device successfully");
}

void VulkanRenderer::CreateSurface()
{
    VkResult result = glfwCreateWindowSurface(instance, window->GetInternalWindow(), nullptr, &surface);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("[VulkanRenderer] Failed to create surface.");
    }
}

bool VulkanRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : extensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void VulkanRenderer::CreateSwapChain()
{
    SwapChainSupportDetails details = QuerySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR format = SelectSwapChainSurfaceFormat(details.formats);
    VkPresentModeKHR presentMode = SelectSwapChainPresentMode(details.presentModes);
    VkExtent2D extent = SelectSwapChainExtent(details.capabilities);

    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
    {
        imageCount = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.presentMode = presentMode;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamily(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentationFamily.value()};

    if (indices.graphicsFamily != indices.presentationFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("[VulkanRenderer] Failed to create swap chain.");
    }

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

    swapchainImageExtent = extent;
    swapchainImageFormat = format;

    Logger::Log(1, "[VulkanRenderer] Created swap chain successfully.");
}

SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

#pragma clang diagnostic pop
