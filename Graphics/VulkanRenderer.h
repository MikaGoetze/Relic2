//
// Created by Mika Goetze on 2019-07-31.
//

#ifndef RELIC_VULKANRENDERER_H
#define RELIC_VULKANRENDERER_H


#include <vulkan/vulkan.h>
#include "Renderer.h"
#include <optional>
#include <vector>

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
    explicit VulkanRenderer(Window *window, bool enableValidation = false);

    ~VulkanRenderer() override;

private:
    VkInstance instance;

    /// Create a Vulkan Instance
    bool CreateInstance();

    /// Check whether a vulkan instance extension is supported.
    /// \param extensionName Name of the extension to query.
    /// \return  Whether or not the extension is available.
    bool ExtensionSupported(const char *extensionName);

    /// Attempt to enable validation layers.
    /// \param createInfo - The Instance creation info struct.
    /// \param layers - The layers to enable.
    void EnableValidationLayers(VkInstanceCreateInfo &createInfo, const std::vector<const char *> &layers);

    /// Check whether a validation layer is supported.
    /// \param validationLayerName - Name of the layer to check.
    /// \return - Whether or not it is supported.
    bool ValidationLayerSupported(const char *validationLayerName);

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
    void InitialiseDebugMessenger();

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
    bool SelectPhysicalDevice();

    /// Create a logical device.
    void CreateLogicalDevice();

    /// Find a queue family from a physical device.
    /// \param device - The physical device
    /// \return The queue family.
    QueueFamilyIndices FindQueueFamily(VkPhysicalDevice device);

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
    int ScorePhysicalDevice(VkPhysicalDevice &device);

    /// Create a surface.
    void CreateSurface();

    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    void CreateSwapChain();

    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR SelectSwapChainSurfaceFormat(std::vector<VkSurfaceFormatKHR> &formats);

    VkPresentModeKHR SelectSwapChainPresentMode(std::vector<VkPresentModeKHR> modes);

    VkExtent2D SelectSwapChainExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    //Variables

    bool validationLayersEnabled;

    std::vector<const char *> *GetRequiredExtensions();

    /// Cache of supported exteions.
    VkExtensionProperties *supportedExtensions;
    /// Number of supported extensions currently cached.
    uint32_t supportedExtensionCount;

    VkLayerProperties *supportedValidationLayers;
    uint32_t supportedValidationLayersCount;

    std::vector<const char *> *enabledValidationLayers;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;

    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentationQueue;

    const std::vector<const char *> layers = {
            "VK_LAYER_LUNARG_standard_validation"
    };

    const std::vector<const char *> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    VkSurfaceFormatKHR swapchainImageFormat;
    VkExtent2D swapchainImageExtent;
};


#endif //RELIC_VULKANRENDERER_H
