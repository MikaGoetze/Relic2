//
// Created by mikag on 3/27/2020.
//

#ifndef RELIC_VULKANUTILS_H
#define RELIC_VULKANUTILS_H

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <glm/vec3.hpp>
#include <Graphics/Model.h>
#include <array>

VkShaderModule CreateShaderModule(const std::vector<char> &code, const VkDevice &device)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule module;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module.");
    }

    return module;
}

void CreateBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VkBuffer &buffer, VmaAllocation &allocation)
{
    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = size;
    createInfo.usage = usageFlags;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = memoryUsage;

    if (vmaCreateBuffer(allocator, &createInfo, &allocationCreateInfo, &buffer, &allocation, nullptr))
    {
        throw std::runtime_error("failed to create buffer");
    }
}

void WriteToBufferDirect(VmaAllocator allocator, VmaAllocation allocation, void *data, size_t size)
{
    void *map;
    if (vmaMapMemory(allocator, allocation, &map))
    {
        throw std::runtime_error("failed to map buffer");
    }

    memcpy(map, data, size);
    vmaUnmapMemory(allocator, allocation);
}

void CopyBuffer(VkBuffer sourceBuffer, VkBuffer destBuffer, VkDeviceSize size, VkCommandPool commandPool, VkDevice device, VkQueue queue)
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = commandPool;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &info);

    VkBufferCopy copy = {};
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = size;
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, destBuffer, 1, &copy);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

VkCommandBuffer StartSingleUseCommandBuffer(VkCommandPool commandPool, VkDevice device)
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = commandPool;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &info);
    return commandBuffer;
}

void EndSingleUseCommandBuffer(VkCommandBuffer buffer, VkQueue queue, VkCommandPool commandPool, VkDevice device)
{
    vkEndCommandBuffer(buffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandPool, 1, &buffer);
}

void CopyBufferToImage(VkBuffer sourceBuffer, VkImage image, VkImageLayout layout, VkExtent3D extent, VkCommandPool commandPool, VkDevice device, VkQueue queue)
{
    VkCommandBuffer buffer = StartSingleUseCommandBuffer(commandPool, device);

    VkBufferImageCopy regions = {};
    regions.bufferRowLength = 0;
    regions.bufferImageHeight = 0;
    regions.bufferOffset = 0;
    regions.imageOffset = {0,0,0};
    regions.imageSubresource = VkImageSubresourceLayers {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        0,
        1
    };
    regions.imageExtent = extent;

    vkCmdCopyBufferToImage(buffer, sourceBuffer, image, layout, 1, &regions);

    EndSingleUseCommandBuffer(buffer, queue, commandPool, device);
}

void WriteToBuffer(VmaAllocator allocator, VkBuffer buffer, void *data, size_t size, VkCommandPool commandPool, VkDevice device, VkQueue queue)
{
    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    CreateBuffer(allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingAllocation);
    WriteToBufferDirect(allocator, stagingAllocation, data, size);

    CopyBuffer(stagingBuffer, buffer, size, commandPool, device, queue);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
}

void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkFormat format, VkCommandPool commandPool, VkDevice device, VkQueue queue)
{
    VkCommandBuffer buffer = StartSingleUseCommandBuffer(commandPool, device);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            1,
            0,
            1
    };

    VkPipelineStageFlags sourceStage, destinationStage;
    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition...");
    }

    vkCmdPipelineBarrier(buffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    EndSingleUseCommandBuffer(buffer, queue, commandPool, device);
}

void WriteToImage(VmaAllocator allocator, VkImage image, VkFormat format, VkImageLayout layout, VkImageLayout endLayout, void* data, size_t size, VkExtent3D extent, VkCommandPool commandPool, VkDevice device, VkQueue queue)
{
    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    CreateBuffer(allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingAllocation);
    WriteToBufferDirect(allocator, stagingAllocation, data, size);

    TransitionImageLayout(image, layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, format, commandPool, device, queue);

    CopyBufferToImage(stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, extent, commandPool, device, queue);

    TransitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, endLayout, format, commandPool, device, queue);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
}

VkVertexInputBindingDescription GetVertexInputBindingDescription()
{
    VkVertexInputBindingDescription description = {};
    description.stride = sizeof(Vertex);
    description.binding = 0;
    description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return description;
}

std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
    //position
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    //normal
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    //tex coord
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, textureCoordinate);

    return attributeDescriptions;
}

void CreateSampler(VkDevice device, VkSampler& sampler)
{
    VkSamplerCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.magFilter = VK_FILTER_LINEAR;
    createInfo.minFilter = VK_FILTER_LINEAR;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.anisotropyEnable = false;
    createInfo.mipLodBias = 0;
    createInfo.compareEnable = false;
    createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    createInfo.unnormalizedCoordinates = false;

    if(vkCreateSampler(device, &createInfo, nullptr, &sampler) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create sampler.");
    }
}


void CreateImage(VmaAllocator allocator, VkImage& image, VmaAllocation& allocation, VkImageType type, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t layers,
                 VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usageFlags)
{
    VkImageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType = type;
    createInfo.format = format;
    createInfo.extent = {width, height, depth};
    createInfo.mipLevels = mipLevels;
    createInfo.arrayLayers = layers;
    createInfo.samples = samples;
    createInfo.usage = usageFlags;
    createInfo.tiling = tiling;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if(vmaCreateImage(allocator, &createInfo, &allocationCreateInfo, &image, &allocation, nullptr))
    {
        throw std::runtime_error("failed to create image.");
    }
}

void CreateImageView(VkDevice device, VkImageView& imageView, VkImage& image, VkFormat format, VkImageViewType viewType, VkImageAspectFlags aspect)
{
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.format = format;
    createInfo.viewType = viewType;
    createInfo.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
    };
    createInfo.subresourceRange = {
            aspect,
            0,
            VK_REMAINING_MIP_LEVELS,
            0,
            VK_REMAINING_ARRAY_LAYERS
    };

    if(vkCreateImageView(device, &createInfo, nullptr, &imageView))
    {
        throw std::runtime_error("failed to create image view.");
    }
}

#endif //RELIC_VULKANUTILS_H
