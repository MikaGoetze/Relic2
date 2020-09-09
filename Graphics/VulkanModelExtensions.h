//
// Created by mikag on 17/08/2020.
//

#ifndef RELIC_VULKANMODELEXTENSIONS_H
#define RELIC_VULKANMODELEXTENSIONS_H

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

struct Buffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct VulkanRenderData
{
    Buffer vertexBuffer;
    Buffer indexBuffer;
    bool ready;
};

struct Image
{
    VkImage image;
    VkSampler sampler;
    VkImageView view;
    VmaAllocation allocation;
};

struct VulkanMaterialData
{
    VkDescriptorSet descriptorSet;
    Image texture;
};

#endif //RELIC_VULKANMODELEXTENSIONS_H
