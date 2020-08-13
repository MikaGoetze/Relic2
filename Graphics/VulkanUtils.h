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

VkShaderModule CreateShaderModule(const std::vector<char>& code, const VkDevice& device)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule module;
    if(vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module.");
    }

    return module;
}

VkVertexInputBindingDescription GetVertexInputBindingDescription()
{
    VkVertexInputBindingDescription description = {};
    description.stride = sizeof(glm::vec3);
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

#endif //RELIC_VULKANUTILS_H
