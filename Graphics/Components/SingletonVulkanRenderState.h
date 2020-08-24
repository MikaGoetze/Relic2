//
// Created by mikag on 24/08/2020.
//

#ifndef RELIC_SINGLETONVULKANRENDERSTATE_H
#define RELIC_SINGLETONVULKANRENDERSTATE_H

#include <vulkan/vulkan.h>
#include <vector>
#include <Libraries/IMGUI/imgui.h>
#include <Graphics/vk_mem_alloc.h>
#include "SingletonRenderState.h"

struct SingletonVulkanRenderState : SingletonRenderState
{
    VkInstance instance;
    uint32_t imageIndex;
    bool validationLayersEnabled;

    VkExtensionProperties *supportedExtensions;
    uint32_t supportedExtensionCount;

    VkLayerProperties *supportedValidationLayers;
    uint32_t supportedValidationLayersCount;

    std::vector<const char *> enabledValidationLayers;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkSurfaceKHR surface{};
    VkRenderPass renderPass{};
    VkDescriptorSetLayout descriptorSetLayout{};
    VkDescriptorPool descriptorPool{};
    std::vector<VkDescriptorSet> descriptorSets;
    VkPipelineLayout pipelineLayout{};
    VkPipeline graphicsPipeline{};

    VkPhysicalDevice physicalDevice{};
    std::vector<VkFramebuffer> swapchainFrameBuffers;
    VkCommandPool commandPool{};
    std::vector<VkCommandBuffer> commandBuffers;

    //TODO: Temp
    ImDrawData *imGuiDrawData;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VmaAllocation> uniformBufferAllocations;

    VkImage depthImage;
    VmaAllocation depthImageAllocation;
    VkImageView depthImageView;

    VkDevice device{};

    VkQueue graphicsQueue{};
    VkQueue presentationQueue{};

    int MAX_FRAMES_IN_FLIGHT = 2;

    std::vector<const char *> layers = {
            "VK_LAYER_LUNARG_standard_validation"
    };

    std::vector<const char *> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkSwapchainKHR swapchain{};
    std::vector<VkImage> swapchainImages;
    VkSurfaceFormatKHR swapchainImageFormat{};
    VkExtent2D swapchainImageExtent{};

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    bool framebufferResized = false;

    VmaAllocator allocator;

    std::vector<VkImageView> swapchainImageViews;
};

struct other
{

};

#endif //RELIC_SINGLETONVULKANRENDERSTATE_H
