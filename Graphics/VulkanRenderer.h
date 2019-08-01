//
// Created by Mika Goetze on 2019-07-31.
//

#ifndef RELIC_VULKANRENDERER_H
#define RELIC_VULKANRENDERER_H


#include <vulkan/vulkan.h>
#include "Renderer.h"

class VulkanRenderer : public Renderer
{
public:
    explicit VulkanRenderer(bool enableValidationLayers);

    ~VulkanRenderer();


private:
    VkInstance instance;

    /// Create a Vulkan Instance
    void CreateInstance(bool enableValidationLayers);

    /// Check whether a vulkan instance extension is supported.
    /// \param extensionName Name of the extension to query.
    /// \return  Whether or not the extension is available.
    bool ExtensionSupported(const char *extensionName);

    void EnableValidationLayers(VkInstanceCreateInfo &createInfo, const std::vector<const char *> &layers);

    bool ValidationLayerSupported(const char *validationLayerName);

    /// Cache of supported exteions.
    VkExtensionProperties *supportedExtensions;
    /// Number of supported extensions currently cached.
    uint32_t supportedExtensionCount;

    VkLayerProperties *supportedValidationLayers;
    uint32_t supportedValidationLayersCount;


    std::vector<const char *> *enabledValidationLayers;

};


#endif //RELIC_VULKANRENDERER_H
