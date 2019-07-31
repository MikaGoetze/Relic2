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
    VulkanRenderer();

    void Initialise() override;

    void Cleanup() override;

private:
    VkInstance instance;

    void CreateInstance();
};


#endif //RELIC_VULKANRENDERER_H
