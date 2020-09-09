#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

#define GLFW_INCLUDE_VULKAN
#define VMA_IMPLEMENTATION
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define VALIDATION_ENABLED true

#include <GLFW/glfw3.h>
#include <Debugging/Logger.h>
#include <cstring>
#include <string>
#include <map>
#include <set>
#include <Core/Util.h>
#include <ResourceManager/ResourceManager.h>
#include "VulkanRenderer.h"
#include "Graphics/VulkanUtils.h"
#include <glm/gtx/quaternion.hpp>
#include "Graphics/VulkanModelExtensions.h"
#include <Libraries/IMGUI/imgui_impl_vulkan.h>
#include <Libraries/IMGUI/imgui_impl_glfw.h>
#include <Core/World.h>
#include <Graphics/Components/SingletonVulkanRenderState.h>
#include <Core/Relic.h>

bool VulkanRenderer::CreateInstance(SingletonVulkanRenderState &state)
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

    std::vector<const char *> *extensions = GetRequiredExtensions(state);

    instanceCreateInfo.enabledExtensionCount = extensions->size();
    instanceCreateInfo.ppEnabledExtensionNames = extensions->data();

    if (state.validationLayersEnabled) EnableValidationLayers(state, instanceCreateInfo);

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &state.instance) != VK_SUCCESS)
    {
        Logger::Log("[VulkanRenderer] Failed to create state.instance...");
        return false;
    }
    Logger::Log("[VulkanRenderer] Created state.instance successfully.");

    delete extensions;
    if (instanceCreateInfo.pNext != nullptr) delete (VkDebugUtilsMessengerCreateInfoEXT *) instanceCreateInfo.pNext;
    return true;
}

bool VulkanRenderer::ExtensionSupported(SingletonVulkanRenderState &state, const char *extensionName)
{
    if (state.supportedExtensions == nullptr)
    {
        //Get extension count first
        vkEnumerateInstanceExtensionProperties(nullptr, &state.supportedExtensionCount, nullptr);
        state.supportedExtensions = new VkExtensionProperties[state.supportedExtensionCount];
        //Then fill the array
        vkEnumerateInstanceExtensionProperties(nullptr, &state.supportedExtensionCount, state.supportedExtensions);
    }

    for (uint32_t i = 0; i < state.supportedExtensionCount; i++)
    {
        if (strcmp(extensionName, state.supportedExtensions[i].extensionName) == 0) return true;
    }
    return false;
}

bool VulkanRenderer::ValidationLayerSupported(SingletonVulkanRenderState &state, const char *validationLayerName)
{
    if (state.supportedValidationLayers == nullptr)
    {
        vkEnumerateInstanceLayerProperties(&state.supportedValidationLayersCount, nullptr);
        state.supportedValidationLayers = new VkLayerProperties[state.supportedValidationLayersCount];
        vkEnumerateInstanceLayerProperties(&state.supportedValidationLayersCount, state.supportedValidationLayers);
    }

    for (uint32_t i = 0; i < state.supportedValidationLayersCount; i++)
    {
        if (strcmp(state.supportedValidationLayers[i].layerName, validationLayerName) == 0) return true;
    }

    return false;
}

void VulkanRenderer::EnableValidationLayers(SingletonVulkanRenderState &state, VkInstanceCreateInfo &createInfo)
{
    for (auto &validationLayer : state.layers)
    {
        if (!ValidationLayerSupported(state, validationLayer))
        {
            Logger::Log("[VulkanRenderer] Validation layer %s is not supported.", validationLayer);
            continue;
        }
        state.enabledValidationLayers.push_back(validationLayer);
        Logger::Log("[VulkanRenderer] Found support for layer: %s", validationLayer);
    }

    createInfo.enabledLayerCount = state.enabledValidationLayers.size();
    createInfo.ppEnabledLayerNames = state.enabledValidationLayers.data();

    auto *debugCreateInfo = new VkDebugUtilsMessengerCreateInfoEXT();
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) debugCreateInfo;
}

std::vector<const char *> *VulkanRenderer::GetRequiredExtensions(SingletonVulkanRenderState &state)
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> desiredExtensions;
    for (int i = 0; i < glfwExtensionCount; i++) desiredExtensions.push_back(glfwExtensions[i]);

    if (state.validationLayersEnabled) desiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    auto *extensions = new std::vector<const char *>();

    //Confirm that all extensions are available.
    for (auto &extension : desiredExtensions)
    {
        if (!ExtensionSupported(state, extension))
        {
            Logger::Log("[VulkanRenderer] Extension not available: %s", extension);
            continue;
        }
        extensions->push_back(extension);
        Logger::Log("[VulkanRenderer] Found support for: %s");
    }

    return extensions;
}

VkBool32 VulkanRenderer::ValidationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                                            const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData)
{
    Logger::Log("[ValidationMessage] %s", callbackData->pMessage);

    return VK_FALSE;
}

void VkErrorCallback(VkResult result)
{
    if (result == VK_SUCCESS) return;
    Logger::Log("[ValidationMessage] %s", result);
    if (result < 0) throw std::runtime_error("encountered error.");
}

void VulkanRenderer::InitialiseDebugMessenger(SingletonVulkanRenderState &state)
{
    if (!state.validationLayersEnabled) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};

    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugMessenger(&state.instance, &createInfo, nullptr, &state.debugMessenger) != VK_SUCCESS)
    {
        Logger::Log("[VulkanRenderer] Failed to create debug messenger");
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
    auto function = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (function != nullptr)
    {
        Logger::Log("[VulkanRenderer] Removed messenger.");
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

bool VulkanRenderer::SelectPhysicalDevice(SingletonVulkanRenderState &state)
{
    uint32_t numDevices;
    vkEnumeratePhysicalDevices(state.instance, &numDevices, nullptr);

    if (numDevices == 0)
    {
        Logger::Log("[VulkanRenderer] Found no physical devices...");
        return false;
    }

    auto *devices = new VkPhysicalDevice[numDevices];
    vkEnumeratePhysicalDevices(state.instance, &numDevices, devices);

    std::multimap<int, VkPhysicalDevice> scoredDevices;

    for (uint32_t i = 0; i < numDevices; i++)
    {
        VkPhysicalDevice &device = devices[i];

        int score = ScorePhysicalDevice(state, device);
        if (score != 0) scoredDevices.insert(std::make_pair(score, device));
    }

    if (scoredDevices.empty()) return false;
    state.physicalDevice = scoredDevices.rbegin()->second;

    return true;
}

int VulkanRenderer::ScorePhysicalDevice(SingletonVulkanRenderState &state, VkPhysicalDevice &device)
{
    int score = 0;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    //Highly score dGPUs.
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;

    score += deviceProperties.limits.maxImageDimension2D;

    //set the device into the state for other functions to refer to
    state.physicalDevice = device;

    //We're going to ignore this for now.
    //    if(!deviceFeatures.geometryShader) score = 0;

    //If we don't support graphics, this device is not appropriate
    QueueFamilyIndices indices = FindQueueFamily(state);

    bool areExtensionsSupported = CheckDeviceExtensionSupport(state);
    bool isSwapchainSupported = false;
    if (areExtensionsSupported)
    {
        SwapChainSupportDetails details = QuerySwapChainSupport(state);
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

QueueFamilyIndices VulkanRenderer::FindQueueFamily(SingletonVulkanRenderState &state)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(state.physicalDevice, &queueFamilyCount, nullptr);
    auto *queueFamilies = new VkQueueFamilyProperties[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(state.physicalDevice, &queueFamilyCount, queueFamilies);

    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentationSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(state.physicalDevice, i, state.surface, &presentationSupport);

        if (presentationSupport)
        {
            indices.presentationFamily = i;
        }

        if (indices.IsComplete()) break;
    }

    delete[] queueFamilies;
    return indices;
}

void VulkanRenderer::CreateLogicalDevice(SingletonVulkanRenderState &state)
{
    QueueFamilyIndices indices = FindQueueFamily(state);

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
    deviceCreateInfo.ppEnabledExtensionNames = state.deviceExtensions.data();
    deviceCreateInfo.enabledExtensionCount = state.deviceExtensions.size();

    if (!state.enabledValidationLayers.empty())
    {
        deviceCreateInfo.enabledLayerCount = state.enabledValidationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = state.enabledValidationLayers.data();
    } else
    {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(state.physicalDevice, &deviceCreateInfo, nullptr, &state.device) != VK_SUCCESS)
    {
        throw std::runtime_error("[VulkanRenderer] Failed to create logical device.");
    }

    vkGetDeviceQueue(state.device, indices.graphicsFamily.value(), 0, &state.graphicsQueue);
    vkGetDeviceQueue(state.device, indices.presentationFamily.value(), 0, &state.presentationQueue);

    Logger::Log("[VulkanRenderer] Created logical device successfully");
}

void VulkanRenderer::CreateSurface(SingletonVulkanRenderState &state)
{
    VkResult result = glfwCreateWindowSurface(state.instance, window->GetInternalWindow(), nullptr, &state.surface);
    if (result != VK_SUCCESS || state.surface == nullptr)
    {
        throw std::runtime_error("[VulkanRenderer] Failed to create state.surface.");
    }
}

bool VulkanRenderer::CheckDeviceExtensionSupport(SingletonVulkanRenderState &state)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(state.physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(state.physicalDevice, nullptr, &extensionCount, extensions.data());

    std::set<std::string> requiredExtensions(state.deviceExtensions.begin(), state.deviceExtensions.end());

    for (const auto &extension : extensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void VulkanRenderer::CreateSwapChain(SingletonVulkanRenderState &state)
{
    SwapChainSupportDetails details = QuerySwapChainSupport(state);

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
    createInfo.surface = state.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.presentMode = presentMode;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamily(state);
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

    if (vkCreateSwapchainKHR(state.device, &createInfo, nullptr, &state.swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("[VulkanRenderer] Failed to create swap chain.");
    }

    vkGetSwapchainImagesKHR(state.device, state.swapchain, &imageCount, nullptr);
    state.swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(state.device, state.swapchain, &imageCount, state.swapchainImages.data());

    state.swapchainImageExtent = extent;
    state.swapchainImageFormat = format;

    Logger::Log("[VulkanRenderer] Created swap chain successfully.");
}

SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(SingletonVulkanRenderState &state)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state.physicalDevice, state.surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(state.physicalDevice, state.surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(state.physicalDevice, state.surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(state.physicalDevice, state.surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(state.physicalDevice, state.surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void VulkanRenderer::CreateSwapchainImageViews(SingletonVulkanRenderState &state)
{
    state.swapchainImageViews.resize(state.swapchainImages.size());

    for (size_t i = 0; i < state.swapchainImageViews.size(); i++)
    {
        CreateImageView(state.device, state.swapchainImageViews[i], state.swapchainImages[i], state.swapchainImageFormat.format, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void VulkanRenderer::CreateGraphicsPipeline(SingletonVulkanRenderState &state)
{
    std::vector<char> vertShaderCode = ReadFile("Shaders/vert.spv");
    std::vector<char> fragShaderCode = ReadFile("Shaders/frag.spv");

    VkShaderModule vertModule = CreateShaderModule(vertShaderCode, state.device);
    VkShaderModule fragModule = CreateShaderModule(fragShaderCode, state.device);

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
    viewport.width = (float) state.swapchainImageExtent.width;
    viewport.height = (float) state.swapchainImageExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = state.swapchainImageExtent;

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

    VkPushConstantRange pushConstants = {};
    pushConstants.offset = 0;
    pushConstants.size = sizeof(PushConstants);
    pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    std::vector<VkDescriptorSetLayout> layouts = {state.descriptorSetLayout, state.materialDescriptorSetLayout};
    pipelineLayoutInfo.setLayoutCount = layouts.size();
    pipelineLayoutInfo.pSetLayouts = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstants;

    if (vkCreatePipelineLayout(state.device, &pipelineLayoutInfo, nullptr, &state.pipelineLayout) != VK_SUCCESS)
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
    pipelineInfo.layout = state.pipelineLayout;
    pipelineInfo.renderPass = state.renderPass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(state.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &state.graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline.");
    }

    vkDestroyShaderModule(state.device, fragModule, nullptr);
    vkDestroyShaderModule(state.device, vertModule, nullptr);
}

void VulkanRenderer::CreateRenderPass(SingletonVulkanRenderState &state)
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = state.swapchainImageFormat.format;
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

    if (vkCreateRenderPass(state.device, &renderPassInfo, nullptr, &state.renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass.");
    }
}

void VulkanRenderer::CreateFrameBuffers(SingletonVulkanRenderState &state)
{
    state.swapchainFrameBuffers.resize(state.swapchainImageViews.size());

    for (size_t i = 0; i < state.swapchainImageViews.size(); i++)
    {
        std::array<VkImageView, 2> attachments = {
                state.swapchainImageViews[i],
                state.depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = state.renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = state.swapchainImageExtent.width;
        framebufferInfo.height = state.swapchainImageExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(state.device, &framebufferInfo, nullptr, &state.swapchainFrameBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer.");
        }
    }
}

void VulkanRenderer::CreateCommandPool(SingletonVulkanRenderState &state)
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamily(state);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(state.device, &poolInfo, nullptr, &state.commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool.");
    }
}

void VulkanRenderer::CreateCommandBuffers(SingletonVulkanRenderState &state, bool isRecreate)
{
    state.commandBuffers.resize(state.swapchainFrameBuffers.size());

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = state.commandPool;
    allocateInfo.commandBufferCount = state.commandBuffers.size();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(state.device, &allocateInfo, state.commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffer");
    }
}

void VulkanRenderer::CreateSynchronisationObjects(SingletonVulkanRenderState &state)
{
    state.imageAvailableSemaphores.resize(state.MAX_FRAMES_IN_FLIGHT);
    state.renderFinishedSemaphores.resize(state.MAX_FRAMES_IN_FLIGHT);
    state.inFlightFences.resize(state.MAX_FRAMES_IN_FLIGHT);
    state.imagesInFlight.resize(state.swapchainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < state.MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(state.device, &semaphoreInfo, nullptr, &state.imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(state.device, &semaphoreInfo, nullptr, &state.renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(state.device, &fenceInfo, nullptr, &state.inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronisation objects for a frame.");
        }
    }
}

void VulkanRenderer::StartFrame(SingletonRenderState &s)
{
    auto & state = (SingletonVulkanRenderState&) s;
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    vkWaitForFences(state.device, 1, &state.inFlightFences[state.currentFrame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(state.device, state.swapchain, UINT64_MAX, state.imageAvailableSemaphores[state.currentFrame], VK_NULL_HANDLE, &state.imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain(state);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swapchain image.");
    }

    if (state.imagesInFlight[state.imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(state.device, 1, &state.imagesInFlight[state.imageIndex], VK_TRUE, UINT64_MAX);
    }

    state.imagesInFlight[state.imageIndex] = state.inFlightFences[state.currentFrame];

    UpdateUniformBuffers(state.imageIndex);
    StartCommandBuffer(state);
}

void VulkanRenderer::CleanupSwapchain(SingletonVulkanRenderState &state)
{
    for (VkFramebuffer framebuffer : state.swapchainFrameBuffers)
    {
        vkDestroyFramebuffer(state.device, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(state.device, state.commandPool, static_cast<uint32_t>(state.commandBuffers.size()), state.commandBuffers.data());

    vkDestroyPipeline(state.device, state.graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(state.device, state.pipelineLayout, nullptr);
    vkDestroyRenderPass(state.device, state.renderPass, nullptr);

    for (auto view : state.swapchainImageViews) vkDestroyImageView(state.device, view, nullptr);

    vkDestroySwapchainKHR(state.device, state.swapchain, nullptr);

    for (size_t i = 0; i < state.swapchainImages.size(); i++)
    {
        vmaDestroyBuffer(state.allocator, state.uniformBuffers[i], state.uniformBufferAllocations[i]);
    }

    vkDestroyImageView(state.device, state.depthImageView, nullptr);
    vmaDestroyImage(state.allocator, state.depthImage, state.depthImageAllocation);

    vkDestroyDescriptorPool(state.device, state.descriptorPool, nullptr);
}


void VulkanRenderer::RecreateSwapChain(SingletonVulkanRenderState &state)
{
    while (window->IsMinimized()) glfwWaitEvents();

    vkDeviceWaitIdle(state.device);
    ImGui::Render();

    CleanupSwapchain(state);
//    ImGui_ImplVulkan_Shutdown();

    CreateSwapChain(state);
    CreateSwapchainImageViews(state);
    CreateRenderPass(state);
    CreateGraphicsPipeline(state);
    CreateDepthResources(state);
    CreateFrameBuffers(state);
    CreateUniformBuffers(state);
    CreateDescriptorSetPool(state);
    CreateDescriptorSets(state);
    CreateCommandBuffers(state, false);
    SetupImGui(state);
}

void VulkanRenderer::WindowResizedCallback(Window *window, int width, int height)
{
    auto *state = reinterpret_cast<SingletonVulkanRenderState *> (window->GetUserPointer());
    state->framebufferResized = true;
}

void VulkanRenderer::CreateAllocator(SingletonVulkanRenderState &state)
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.device = state.device;
    allocatorInfo.physicalDevice = state.physicalDevice;
    allocatorInfo.instance = state.instance;

    if (vmaCreateAllocator(&allocatorInfo, &state.allocator) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create VMA Allocator.");
    }
}

void VulkanRenderer::CreateDescriptorSetLayout(SingletonVulkanRenderState &state)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {uboLayoutBinding};

    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = bindings.size();
    createInfo.pBindings = bindings.data();
    createInfo.flags = 0;

    if (vkCreateDescriptorSetLayout(state.device, &createInfo, nullptr, &state.descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout.");
    }

    VkDescriptorSetLayoutBinding materialLayoutBinding = {};
    materialLayoutBinding.binding = 1;
    materialLayoutBinding.descriptorCount = 1;
    materialLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    bindings = {materialLayoutBinding};

    createInfo.bindingCount = bindings.size();
    createInfo.pBindings = bindings.data();

    if(vkCreateDescriptorSetLayout(state.device, &createInfo, nullptr, &state.materialDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create material descriptor set layout.");
    }
}

void VulkanRenderer::CreateUniformBuffers(SingletonVulkanRenderState &state)
{
    //Create Uniform buffers
    state.uniformBuffers.resize(state.swapchainImages.size());
    state.uniformBufferAllocations.resize(state.swapchainImages.size());
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    for (int i = 0; i < state.swapchainImages.size(); i++)
    {
        CreateBuffer(state.allocator, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, state.uniformBuffers[i], state.uniformBufferAllocations[i]);
    }
}

void VulkanRenderer::UpdateUniformBuffers(uint32_t currentImage)
{
//    static auto startTime = std::chrono::high_resolution_clock::now();
//    auto currentTime = std::chrono::high_resolution_clock::now();
//
//    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
//
//    UniformBufferObject ubo = {};
//    ubo.model = glm::rotate(glm::rotate(glm::identity<glm::mat4>(), time * glm::radians(45.0f), glm::vec3(0, 1, 0)), glm::radians(-90.0f), glm::vec3(1, 0, 0));
//    ubo.view = glm::lookAt(glm::vec3(30), glm::zero<glm::vec3>(), glm::vec3(0, 1, 0));
//    ubo.proj = glm::perspective(glm::radians(45.0f), swapchainImageExtent.width / (float) swapchainImageExtent.height, 1.0f, 200.0f);
//
//    //counteract opengl flip
//    ubo.proj[1][1] *= -1;
//
//    WriteToBufferDirect(allocator, uniformBufferAllocations[currentImage], &ubo, sizeof(UniformBufferObject));
}

void VulkanRenderer::CreateDescriptorSetPool(SingletonVulkanRenderState &state)
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
    if (vkCreateDescriptorPool(state.device, &pool_info, nullptr, &state.descriptorPool))
    {
        throw std::runtime_error("failed to create descriptor pool.");
    }
}

void VulkanRenderer::CreateDescriptorSets(SingletonVulkanRenderState &state)
{
    std::vector<VkDescriptorSetLayout> layouts(state.swapchainImages.size(), state.descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = state.descriptorPool;
    allocateInfo.descriptorSetCount = state.swapchainImages.size();
    allocateInfo.pSetLayouts = layouts.data();

    state.descriptorSets.resize(state.swapchainImages.size());
    if (vkAllocateDescriptorSets(state.device, &allocateInfo, state.descriptorSets.data()))
    {
        throw std::runtime_error("failed to allocate descriptor sets.");
    }

    for (size_t i = 0; i < state.swapchainImages.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = state.uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = state.descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(state.device, 1, &descriptorWrite, 0, nullptr);
    }
}

void VulkanRenderer::CreateDepthResources(SingletonVulkanRenderState &state)
{
    CreateImage(state.allocator, state.depthImage, state.depthImageAllocation, VK_IMAGE_TYPE_2D, VK_FORMAT_D32_SFLOAT, state.swapchainImageExtent.width, state.swapchainImageExtent.height, 1,
                1, 1,
                VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    CreateImageView(state.device, state.depthImageView, state.depthImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanRenderer::SetupImGui(SingletonVulkanRenderState &state)
{
    ImGui_ImplGlfw_InitForVulkan(window->GetInternalWindow(), false);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = state.instance;
    initInfo.PhysicalDevice = state.physicalDevice;
    initInfo.Device = state.device;
    initInfo.QueueFamily = FindQueueFamily(state).graphicsFamily.value();
    initInfo.Queue = state.graphicsQueue;
    initInfo.PipelineCache = nullptr;
    initInfo.DescriptorPool = state.descriptorPool;
    initInfo.Allocator = nullptr;
    initInfo.MinImageCount = state.MAX_FRAMES_IN_FLIGHT;
    initInfo.ImageCount = state.MAX_FRAMES_IN_FLIGHT;
    initInfo.CheckVkResultFn = &VkErrorCallback;
    ImGui_ImplVulkan_Init(&initInfo, state.renderPass);

    // Use any command queue
    VkCommandPool command_pool = state.commandPool;
    VkCommandBuffer command_buffer = state.commandBuffers[state.currentFrame];

    if (VkResult err = vkResetCommandPool(state.device, command_pool, 0))
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
    if (vkQueueSubmit(state.graphicsQueue, 1, &end_info, VK_NULL_HANDLE))
    {
        throw std::runtime_error("failed to submit queue.");
    }

    if (vkDeviceWaitIdle(state.device))
    {
        throw std::runtime_error("failed to wait for device to become idle.");
    }
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void VulkanRenderer::StartCommandBuffer(SingletonVulkanRenderState &state)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(state.commandBuffers[state.imageIndex], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording a command buffer.");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = state.renderPass;
    renderPassInfo.framebuffer = state.swapchainFrameBuffers[state.imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = state.swapchainImageExtent;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0] = {0.0f, 0.0f, 0.0f, 0.0f};
    clearValues[1] = {1.0f, 0};

    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(state.commandBuffers[state.imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(state.commandBuffers[state.imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, state.graphicsPipeline);

    vkCmdBindDescriptorSets(state.commandBuffers[state.imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipelineLayout, 0, 1, &state.descriptorSets[state.imageIndex], 0, nullptr);
}

void VulkanRenderer::PrepareMesh(SingletonRenderState &s, Mesh &mesh)
{
    auto & state = (SingletonVulkanRenderState&) s;
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
        return;
    }

    //Create buffers
    CreateBuffer(state.allocator, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                 renderData->indexBuffer.buffer,
                 renderData->indexBuffer.allocation);
    CreateBuffer(state.allocator, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                 renderData->vertexBuffer.buffer,
                 renderData->vertexBuffer.allocation);


    //Write data to them
    WriteToBuffer(state.allocator, renderData->indexBuffer.buffer, mesh.indices, indexBufferSize, state.commandPool, state.device, state.graphicsQueue);
    WriteToBuffer(state.allocator, renderData->vertexBuffer.buffer, mesh.vertices, vertexBufferSize, state.commandPool, state.device, state.graphicsQueue);

    renderData->ready = true;

    mesh.renderData = renderData;
}

void VulkanRenderer::CleanupMesh(SingletonRenderState &s, Mesh &mesh)
{
    auto & state = (SingletonVulkanRenderState&) s;
    vkDeviceWaitIdle(state.device);

    auto renderData = (VulkanRenderData *) mesh.renderData;

    vmaDestroyBuffer(state.allocator, renderData->indexBuffer.buffer, renderData->indexBuffer.allocation);
    vmaDestroyBuffer(state.allocator, renderData->vertexBuffer.buffer, renderData->vertexBuffer.allocation);

    delete renderData;
    mesh.renderData = nullptr;
}

void VulkanRenderer::RenderMesh(SingletonRenderState &s, Mesh &mesh, Material &material, TransformComponent transform)
{
    auto & state = (SingletonVulkanRenderState&) s;
    auto renderData = (VulkanRenderData *) mesh.renderData;
    auto matRenderData = (VulkanMaterialData *) material.renderData;
    if (renderData == nullptr || matRenderData == nullptr || !renderData->ready) return;

    VkDeviceSize offset = 0;

    PushConstants pushConstants = {};
    glm::mat4 model = glm::scale(glm::identity<glm::mat4>(), transform.scale);
    model = glm::toMat4(transform.rotation) * model;
    model = glm::translate(model, transform.position);
    pushConstants.mvp = vpMatrix * model;

    vkCmdPushConstants(state.commandBuffers[state.imageIndex], state.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pushConstants);

    vkCmdBindVertexBuffers(state.commandBuffers[state.imageIndex], 0, 1, &renderData->vertexBuffer.buffer, &offset);
    vkCmdBindIndexBuffer(state.commandBuffers[state.imageIndex], renderData->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(state.commandBuffers[state.imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipelineLayout, 1, 1, &matRenderData->descriptorSet, 0, nullptr);

    vkCmdDrawIndexed(state.commandBuffers[state.imageIndex], mesh.indexCount, 1, 0, 0, 0);
}

void VulkanRenderer::EndFrame(SingletonRenderState &s)
{
    auto & state = (SingletonVulkanRenderState&) s;
    //Draw ImGUI, and finish command buffer recording.
    ImGui::Render();
    state.imGuiDrawData = ImGui::GetDrawData();
    if (state.imGuiDrawData != nullptr) ImGui_ImplVulkan_RenderDrawData(state.imGuiDrawData, state.commandBuffers[state.imageIndex]);

    EndCommandBuffer(state.commandBuffers[state.imageIndex]);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {state.imageAvailableSemaphores[state.currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &state.commandBuffers[state.imageIndex];

    VkSemaphore signalSemaphores[] = {state.renderFinishedSemaphores[state.currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(state.device, 1, &state.inFlightFences[state.currentFrame]);

    if (vkQueueSubmit(state.graphicsQueue, 1, &submitInfo, state.inFlightFences[state.currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer.");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = {state.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &state.imageIndex;

    VkResult result = vkQueuePresentKHR(state.presentationQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || state.framebufferResized)
    {
        state.framebufferResized = false;
        RecreateSwapChain(state);
    } else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swapchain image.");
    }

    state.currentFrame = (state.currentFrame + 1) % state.MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::EndCommandBuffer(VkCommandBuffer &commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer.");
    }
}

void VulkanRenderer::Tick(World &world)
{
}

SystemRegistrar VulkanRenderer::registrar(new VulkanRenderer());

void VulkanRenderer::Init(World &world)
{
    Renderer::Init(world);

    //Create our singleton component
    auto registry = world.Registry();
    //Callback to initialise components, also allows us to destroy them neatly.
    registry->on_construct<SingletonVulkanRenderState>().connect<&VulkanRenderer::OnRendererCreation>(this);
    registry->on_destroy<SingletonVulkanRenderState>().connect<&VulkanRenderer::OnRendererDestruction>(this);

    auto entity = registry->create();
    auto &state = registry->emplace<SingletonVulkanRenderState>(entity);
    //Set it into the registry as a context variable
    registry->set<SingletonRenderState *>(&state);
}

void VulkanRenderer::Shutdown(World &world)
{

}

void VulkanRenderer::OnRendererCreation(entt::registry &registry, entt::entity entity)
{
    auto &state = registry.get<SingletonVulkanRenderState>(entity);

    state.instance = VkInstance{};
    state.supportedExtensionCount = 0;
    state.supportedExtensions = nullptr;
    state.supportedValidationLayers = nullptr;
    state.supportedValidationLayersCount = 0;
    state.validationLayersEnabled = VALIDATION_ENABLED;
    state.debugMessenger = {};
    state.imGuiDrawData = nullptr;

    int vulkanSupported = glfwVulkanSupported();
    if (vulkanSupported == GLFW_FALSE)
    {
        Logger::Log("Vulkan is not supported.");
        exit(0);
    }

    window->SetUserPointer(&state);
    window->RegisterWindowSizeChangedCallback(WindowResizedCallback);

    //Try to create an state.instance, otherwise exit.
    if (!CreateInstance(state)) exit(0);
    InitialiseDebugMessenger(state);

    CreateSurface(state);

    if (!SelectPhysicalDevice(state))
    {
        Logger::Log("[VulkanRenderer] Failed to pick physical device.");
    } else
    {
        Logger::Log("[VulkanRenderer] Selected physical device.");
    }

    CreateLogicalDevice(state);

    CreateAllocator(state);
    CreateSwapChain(state);
    CreateSwapchainImageViews(state);
    CreateRenderPass(state);
    CreateDescriptorSetLayout(state);
    CreateGraphicsPipeline(state);
    CreateDepthResources(state);
    CreateFrameBuffers(state);
    CreateCommandPool(state);
    CreateUniformBuffers(state);
    CreateDescriptorSetPool(state);
    CreateDescriptorSets(state);

    CreateCommandBuffers(state, false);
    CreateSynchronisationObjects(state);

    SetupImGui(state);
}

void VulkanRenderer::OnRendererDestruction(entt::registry &registry, entt::entity entity)
{
    auto &state = registry.get<SingletonVulkanRenderState>(entity);

    vkDeviceWaitIdle(state.device);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    CleanupSwapchain(state);

    vkDestroyDescriptorSetLayout(state.device, state.descriptorSetLayout, nullptr);

    vmaDestroyAllocator(state.allocator);

    for (size_t i = 0; i < state.MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(state.device, state.renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(state.device, state.imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(state.device, state.inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(state.device, state.commandPool, nullptr);

    delete state.supportedExtensions;
    delete state.supportedValidationLayers;
    DestroyDebugMessenger(state.instance, state.debugMessenger, nullptr);

    vkDestroyDevice(state.device, nullptr);
    vkDestroySurfaceKHR(state.instance, state.surface, nullptr);
    vkDestroyInstance(state.instance, nullptr);
}

void VulkanRenderer::RegisterMaterial(Material *material)
{
    Renderer::RegisterMaterial(material);
    auto *data = new VulkanMaterialData();

    //Create image and relevant descriptor set.
    auto * state = (SingletonVulkanRenderState*) Relic::Instance()->GetPrimaryWorld()->Registry()->ctx<SingletonRenderState*>();

    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    CreateImage(state->allocator, data->texture.image, data->texture.allocation, VK_IMAGE_TYPE_2D, imageFormat, material->texture->width, material->texture->height, 1, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    CreateImageView(state->device, data->texture.view, data->texture.image, imageFormat, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

    VkExtent3D extent = {
            material->texture->width,
            material->texture->height,
            1
    };

    WriteToImage(state->allocator, data->texture.image, imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, material->texture->data, material->texture->dataSize, extent, state->commandPool, state->device, state->graphicsQueue);

    CreateSampler(state->device, data->texture.sampler);

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = state->descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &state->materialDescriptorSetLayout;

    if(vkAllocateDescriptorSets(state->device, &allocateInfo, &data->descriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set for material.");
    }

    VkDescriptorImageInfo info = {};
    info.sampler = data->texture.sampler;
    info.imageView = data->texture.view;
    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = data->descriptorSet;
    descriptorWrite.dstBinding = 1;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &info;

    vkUpdateDescriptorSets(state->device, 1, &descriptorWrite, 0, nullptr);

    material->renderData = data;
}


#pragma clang diagnostic pop