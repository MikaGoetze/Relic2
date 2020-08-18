//
// Created by Mika Goetze on 2019-07-31.
//

#define GLFW_INCLUDE_VULKAN
#define VMA_IMPLEMENTATION
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>
#include <Debugging/Logger.h>
#include <cstring>
#include <string>
#include <map>
#include <set>
#include <Core/Util.h>
#include <ResourceManager/ResourceManager.h>
#include "VulkanRenderer.h"
#include "VulkanUtils.h"
#include <glm/gtc/matrix_transform.hpp>
#include "VulkanModelExtensions.h"


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

VulkanRenderer::VulkanRenderer(Window *window, Model *model, bool enableValidation) : Renderer(window)
{
    instance = VkInstance{};
    supportedExtensionCount = 0;
    supportedExtensions = nullptr;
    supportedValidationLayers = nullptr;
    supportedValidationLayersCount = 0;
    enabledValidationLayers = new std::vector<const char *>();
    validationLayersEnabled = enableValidation;
    debugMessenger = {};
    imGuiDrawData = nullptr;
    this->model = model;

    int vulkanSupported = glfwVulkanSupported();
    if (vulkanSupported == GLFW_FALSE)
    {
        Logger::Log("Vulkan is not supported.");
        exit(0);
    }

    window->SetUserPointer(this);
    window->RegisterWindowSizeChangedCallback(WindowResizedCallback);

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

    //Create the allocator
    CreateAllocator();

    CreateSwapChain();
    CreateSwapchainImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateDepthResources();
    CreateFrameBuffers();
    CreateCommandPool();
    CreateUniformBuffers();
    CreateDescriptorSetPool();
    CreateDescriptorSets();

    CreateCommandBuffers(false);
    CreateSynchronisationObjects();

    SetupImGui();
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
    CleanupSwapchain();

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    VulkanRenderer::DestroyModel(*model);

    ImGui_ImplVulkan_Shutdown();

    vmaDestroyAllocator(allocator);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    delete supportedExtensions;
    delete supportedValidationLayers;
    delete enabledValidationLayers;
    DestroyDebugMessenger(instance, debugMessenger, nullptr);

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

void VkErrorCallback(VkResult result)
{
    if (result == VK_SUCCESS) return;
    Logger::Log(2, "[ValidationMessage] ", result);
    if (result < 0) throw std::runtime_error("encountered error.");
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
    bool isSwapchainSupported = false;
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

VkPresentModeKHR VulkanRenderer::SelectSwapChainPresentMode(const std::vector<VkPresentModeKHR> &modes)
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
    if (result != VK_SUCCESS || surface == nullptr)
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

void VulkanRenderer::CreateSwapchainImageViews()
{
    swapchainImageViews.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainImageViews.size(); i++)
    {
        CreateImageView(device, swapchainImageViews[i], swapchainImages[i], swapchainImageFormat.format, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void VulkanRenderer::CreateGraphicsPipeline()
{
    std::vector<char> vertShaderCode = ReadFile("Shaders/vert.spv");
    std::vector<char> fragShaderCode = ReadFile("Shaders/frag.spv");

    VkShaderModule vertModule = CreateShaderModule(vertShaderCode, device);
    VkShaderModule fragModule = CreateShaderModule(fragShaderCode, device);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertModule;
    vertShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragModule;
    fragShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    auto bindingDescription = GetVertexInputBindingDescription();
    auto attributeDescriptions = GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapchainImageExtent.width;
    viewport.height = (float) swapchainImageExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapchainImageExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasClamp = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout.");
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline.");
    }

    vkDestroyShaderModule(device, fragModule, nullptr);
    vkDestroyShaderModule(device, vertModule, nullptr);
}

void VulkanRenderer::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapchainImageFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass.");
    }
}

void VulkanRenderer::CreateFrameBuffers()
{
    swapchainFrameBuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); i++)
    {
        std::array<VkImageView, 2> attachments = {
                swapchainImageViews[i],
                depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchainImageExtent.width;
        framebufferInfo.height = swapchainImageExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchainFrameBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer.");
        }
    }
}

void VulkanRenderer::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamily(physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool.");
    }
}

void VulkanRenderer::CreateCommandBuffers(bool isRecreate)
{
    if(!isRecreate) PrepareModel(*model);

    commandBuffers.resize(swapchainFrameBuffers.size());

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = commandPool;
    allocateInfo.commandBufferCount = commandBuffers.size();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffer");
    }
}

void VulkanRenderer::CreateSynchronisationObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronisation objects for a frame.");
        }
    }
}

void VulkanRenderer::Render()
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swapchain image.");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    UpdateUniformBuffers(imageIndex);
    UpdateCommandBuffer(imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer.");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentationQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        RecreateSwapChain();
    } else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swapchain image.");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::FinishPendingRenderingOperations()
{
    vkDeviceWaitIdle(device);
}

void VulkanRenderer::CleanupSwapchain()
{
    for (VkFramebuffer framebuffer : swapchainFrameBuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (auto view : swapchainImageViews) vkDestroyImageView(device, view, nullptr);

    vkDestroySwapchainKHR(device, swapchain, nullptr);

    for (size_t i = 0; i < swapchainImages.size(); i++)
    {
        vmaDestroyBuffer(allocator, uniformBuffers[i], uniformBufferAllocations[i]);
    }

    vkDestroyImageView(device, depthImageView, nullptr);
    vmaDestroyImage(allocator, depthImage, depthImageAllocation);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}


void VulkanRenderer::RecreateSwapChain()
{
    while (window->IsMinimized()) glfwWaitEvents();

    vkDeviceWaitIdle(device);

    CleanupSwapchain();
    ImGui_ImplVulkan_Shutdown();

    CreateSwapChain();
    CreateSwapchainImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateDepthResources();
    CreateFrameBuffers();
    CreateUniformBuffers();
    CreateDescriptorSetPool();
    CreateDescriptorSets();
    CreateCommandBuffers(false);
    SetupImGui();
}

void VulkanRenderer::WindowResizedCallback(Window *window, int width, int height)
{
    auto *renderer = reinterpret_cast<VulkanRenderer *> (window->GetUserPointer());
    renderer->framebufferResized = true;
}

void VulkanRenderer::CreateAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.device = device;
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.instance = instance;

    if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create VMA Allocator.");
    }
}

void VulkanRenderer::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &uboLayoutBinding;
    createInfo.pBindings = &uboLayoutBinding;
    createInfo.flags = 0;

    if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorSetLayout))
    {
        throw std::runtime_error("failed to create descriptor set layout.");
    }
}

void VulkanRenderer::CreateUniformBuffers()
{
    //Create Uniform buffers
    uniformBuffers.resize(swapchainImages.size());
    uniformBufferAllocations.resize(swapchainImages.size());
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    for (int i = 0; i < swapchainImages.size(); i++)
    {
        CreateBuffer(allocator, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, uniformBuffers[i], uniformBufferAllocations[i]);
    }
}

void VulkanRenderer::UpdateUniformBuffers(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo = {};
    ubo.model = glm::rotate(glm::rotate(glm::identity<glm::mat4>(), time * glm::radians(45.0f), glm::vec3(0, 1, 0)), glm::radians(-90.0f), glm::vec3(1, 0, 0));
    ubo.view = glm::lookAt(glm::vec3(30), glm::zero<glm::vec3>(), glm::vec3(0, 1, 0));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapchainImageExtent.width / (float) swapchainImageExtent.height, 1.0f, 200.0f);

    //counteract opengl flip
    ubo.proj[1][1] *= -1;

    WriteToBufferDirect(allocator, uniformBufferAllocations[currentImage], &ubo, sizeof(UniformBufferObject));
}

void VulkanRenderer::CreateDescriptorSetPool()
{
    VkDescriptorPoolSize pool_sizes[] =
            {
                    {VK_DESCRIPTOR_TYPE_SAMPLER,                1000},
                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000}
            };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t) IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool))
    {
        throw std::runtime_error("failed to create descriptor pool.");
    }
}

void VulkanRenderer::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(swapchainImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = descriptorPool;
    allocateInfo.descriptorSetCount = swapchainImages.size();
    allocateInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapchainImages.size());
    if (vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets.data()))
    {
        throw std::runtime_error("failed to allocate descriptor sets.");
    }

    for (size_t i = 0; i < swapchainImages.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

void VulkanRenderer::CreateDepthResources()
{
    CreateImage(allocator, depthImage, depthImageAllocation, VK_IMAGE_TYPE_2D, VK_FORMAT_D32_SFLOAT, swapchainImageExtent.width, swapchainImageExtent.height, 1,
                1, 1,
                VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    CreateImageView(device, depthImageView, depthImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanRenderer::SetupImGui()
{
    ImGui_ImplGlfw_InitForVulkan(window->GetInternalWindow(), false);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = physicalDevice;
    initInfo.Device = device;
    initInfo.QueueFamily = FindQueueFamily(physicalDevice).graphicsFamily.value();
    initInfo.Queue = graphicsQueue;
    initInfo.PipelineCache = nullptr;
    initInfo.DescriptorPool = descriptorPool;
    initInfo.Allocator = nullptr;
    initInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    initInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
    initInfo.CheckVkResultFn = &VkErrorCallback;
    ImGui_ImplVulkan_Init(&initInfo, renderPass);

    // Use any command queue
    VkCommandPool command_pool = commandPool;
    VkCommandBuffer command_buffer = commandBuffers[currentFrame];

    if (VkResult err = vkResetCommandPool(device, command_pool, 0))
    {
        throw std::runtime_error("failed to reset command pool.");
    }

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(command_buffer, &begin_info))
    {
        throw std::runtime_error("failed to start command buffer.");
    }

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &command_buffer;

    if (vkEndCommandBuffer(command_buffer))
    {
        throw std::runtime_error("failed to end command buffer recording.");
    }
    if (vkQueueSubmit(graphicsQueue, 1, &end_info, VK_NULL_HANDLE))
    {
        throw std::runtime_error("failed to submit queue.");
    }

    if (vkDeviceWaitIdle(device))
    {
        throw std::runtime_error("failed to wait for device to become idle.");
    }
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void VulkanRenderer::UpdateCommandBuffer(uint32_t frame)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[frame], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording a command buffer.");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapchainFrameBuffers[frame];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchainImageExtent;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0] = {0.0f, 0.0f, 0.0f, 0.0f};
    clearValues[1] = {1.0f, 0};

    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[frame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[frame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkDeviceSize offsets[1] = {0};

    vkCmdBindDescriptorSets(commandBuffers[frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[frame], 0, nullptr);

    for (size_t m = 0; m < model->meshCount; m++)
    {
        auto mesh = model->meshes[m];
        auto renderData = (VulkanRenderData *) mesh.renderData;

        if (renderData == nullptr || !renderData->ready) continue;

        vkCmdBindVertexBuffers(commandBuffers[frame], 0, 1, &renderData->vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffers[frame], renderData->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffers[frame], mesh.indexCount, 1, 0, 0, 0);
    }

    //Dear IMGUI
    ImGui::Render();
    imGuiDrawData = ImGui::GetDrawData();
    if (imGuiDrawData != nullptr) ImGui_ImplVulkan_RenderDrawData(imGuiDrawData, commandBuffers[frame]);

    vkCmdEndRenderPass(commandBuffers[frame]);

    if (vkEndCommandBuffer(commandBuffers[frame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer.");
    }
}

void VulkanRenderer::PrepareModel(Model &model)
{
    for (size_t i = 0; i < model.meshCount; i++)
    {
        Mesh &mesh = model.meshes[i];

        auto renderData = new VulkanRenderData();
        renderData->indexBuffer = {};
        renderData->vertexBuffer = {};

        VkDeviceSize vertexBufferSize = mesh.vertexCount * sizeof(Vertex);
        VkDeviceSize indexBufferSize = mesh.indexCount * sizeof(uint32_t);

        if (vertexBufferSize == 0 || indexBufferSize == 0)
        {
            //empty mesh?
            renderData->vertexBuffer.buffer = VK_NULL_HANDLE;
            renderData->ready = false;

            mesh.renderData = renderData;
            continue;
        }

        //Create buffers
        CreateBuffer(allocator, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                     renderData->indexBuffer.buffer,
                     renderData->indexBuffer.allocation);
        CreateBuffer(allocator, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                     renderData->vertexBuffer.buffer,
                     renderData->vertexBuffer.allocation);


        //Write data to them
        WriteToBuffer(allocator, renderData->indexBuffer.buffer, mesh.indices, indexBufferSize, commandPool, device, graphicsQueue);
        WriteToBuffer(allocator, renderData->vertexBuffer.buffer, mesh.vertices, vertexBufferSize, commandPool, device, graphicsQueue);

        renderData->ready = true;

        mesh.renderData = renderData;
    }
}

void VulkanRenderer::DestroyModel(Model &model)
{
    for (size_t i = 0; i < model.meshCount; i++)
    {
        Mesh &mesh = model.meshes[i];
        auto renderData = (VulkanRenderData *) mesh.renderData;

        vmaDestroyBuffer(allocator, renderData->indexBuffer.buffer, renderData->indexBuffer.allocation);
        vmaDestroyBuffer(allocator, renderData->vertexBuffer.buffer, renderData->vertexBuffer.allocation);

        delete renderData;
        mesh.renderData = nullptr;
    }
}
