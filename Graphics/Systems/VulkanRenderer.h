//
// Created by Mika Goetze on 2019-07-31.
//

#ifndef RELIC_VULKANRENDERER_H
#define RELIC_VULKANRENDERER_H


#include <vulkan/vulkan.h>
#include "Renderer.h"
#include "Graphics/vk_mem_alloc.h"
#include <optional>
#include <vector>
#include "Graphics/OpenFBX/ofbx.h"
#include "Graphics/Model.h"
#include <glm/glm.hpp>
#include <Libraries/IMGUI/imgui.h>
#include <Core/Components/TransformComponent.h>
#include <Graphics/Components/SingletonVulkanRenderState.h>

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentationFamily;

    bool IsComplete()
    {
        return graphicsFamily.has_value() && presentationFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanRenderer : public Renderer
{
public:
    void Init(World &world) override;

    void Shutdown(World &world) override;

private:
    static SystemRegistrar registrar;


    /// Create a Vulkan Instance
    bool CreateInstance(SingletonVulkanRenderState &state);

    void CreateUniformBuffers(SingletonVulkanRenderState &state);

    /// Check whether a vulkan instance extension is supported.
    /// \param extensionName Name of the extension to query.
    /// \return  Whether or not the extension is available.
    bool ExtensionSupported(SingletonVulkanRenderState &state, const char *extensionName);

    /// Attempt to enable validation layers.
    /// \param createInfo - The Instance creation info struct.
    /// \param layers - The layers to enable.
    void EnableValidationLayers(SingletonVulkanRenderState &state, VkInstanceCreateInfo &createInfo, const std::vector<const char *> &layers);

    /// Check whether a validation layer is supported.
    /// \param validationLayerName - Name of the layer to check.
    /// \return - Whether or not it is supported.
    bool ValidationLayerSupported(SingletonVulkanRenderState &state, const char *validationLayerName);

    /// Validation layer callback.
    /// \param messageSeverity - Severity of the message
    /// \param messageType - The type of the message.
    /// \param callbackData - The data passed back from validation.
    /// \param userData - Any user data.
    /// \return
    static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                             const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
                                                             void *userData);

    /// Attach the debug messenger to vulkan.
    void InitialiseDebugMessenger(SingletonVulkanRenderState &state);

    /// Create the debug messenger.
    /// \param instance - The VkInstance to attach to.
    /// \param createInfo - The messenger create info
    /// \param allocatorCallback - The allocator callback (if using)
    /// \param messenger - The messenger to user.
    /// \return - Success.
    VkResult CreateDebugMessenger(VkInstance *instance, const VkDebugUtilsMessengerCreateInfoEXT *createInfo, const
    VkAllocationCallbacks *allocatorCallback, VkDebugUtilsMessengerEXT *messenger);

    /// Select a suitable physical device.
    /// \return Whether or not it was succesfull.
    bool SelectPhysicalDevice(SingletonVulkanRenderState &state);

    /// Create a logical device.
    void CreateLogicalDevice(SingletonVulkanRenderState &state);

    /// Find a queue family from a physical device.
    /// \param device - The physical device
    /// \return The queue family.
    QueueFamilyIndices FindQueueFamily(SingletonVulkanRenderState &state);

    /// Setup the debug messenger create info.
    /// \param createInfo - The create info to init.
    static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    /// Cleanup the debug messenger.
    /// \param instance - The VkInstance it was attached to
    /// \param messenger - The messenger we're cleaning up
    /// \param callback - The allocation callback.
    static void DestroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks *
    callback);

    /// Score a physical device based on it's features.
    /// \param device - The device to score.
    /// \return The score of the device (higher is better)
    int ScorePhysicalDevice(SingletonVulkanRenderState &state, VkPhysicalDevice &device);

    /// Create a surface.
    void CreateSurface(SingletonVulkanRenderState &state);

public:
    void RenderMesh(SingletonRenderState &state, Mesh &mesh, TransformComponent transform) override;
    void EndFrame(SingletonRenderState &state) override;
    void StartFrame(SingletonRenderState &state) override;
    void PrepareMesh(SingletonRenderState &state, Mesh &mesh) override;
    void CleanupMesh(SingletonRenderState &state, Mesh &mesh) override;

private:
    void OnRendererCreation(entt::registry& registry, entt::entity entity);
    void OnRendererDestruction(entt::registry& registry, entt::entity entity);

    std::vector<const char *> *GetRequiredExtensions(SingletonVulkanRenderState &state);

    bool CheckDeviceExtensionSupport(SingletonVulkanRenderState &state);

    void CreateSwapChain(SingletonVulkanRenderState &state);

    SwapChainSupportDetails QuerySwapChainSupport(SingletonVulkanRenderState &state);

    VkSurfaceFormatKHR SelectSwapChainSurfaceFormat(std::vector<VkSurfaceFormatKHR> &formats);

    VkPresentModeKHR SelectSwapChainPresentMode(const std::vector<VkPresentModeKHR>& modes);

    VkExtent2D SelectSwapChainExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    void CreateSwapchainImageViews(SingletonVulkanRenderState &state);

    void CreateDepthResources(SingletonVulkanRenderState &state);

    void CreateDescriptorSetLayout(SingletonVulkanRenderState &state);
    void CreateGraphicsPipeline(SingletonVulkanRenderState &state);

    void CreateRenderPass(SingletonVulkanRenderState &state);

    static void WindowResizedCallback(Window* window, int width, int height);

    void CreateFrameBuffers(SingletonVulkanRenderState &state);

    void CreateCommandPool(SingletonVulkanRenderState &state);
    void CreateDescriptorSetPool(SingletonVulkanRenderState &state);
    void CreateDescriptorSets(SingletonVulkanRenderState &state);

    void CreateCommandBuffers(SingletonVulkanRenderState &state, bool isRecreate);

    void CreateSynchronisationObjects(SingletonVulkanRenderState &state);

    void RecreateSwapChain(SingletonVulkanRenderState &state);

    void CleanupSwapchain(SingletonVulkanRenderState &state);

    void CreateAllocator(SingletonVulkanRenderState &state);

    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct PushConstants
    {
        glm::mat4 mvp;
    };
public:
    void Tick(World& world) override;

private:
    void UpdateUniformBuffers(uint32_t currentImage);

    void SetupImGui(SingletonVulkanRenderState &state);

    void StartCommandBuffer(SingletonVulkanRenderState &state);
    void EndCommandBuffer(VkCommandBuffer &commandBuffer);
};


#endif //RELIC_VULKANRENDERER_H
