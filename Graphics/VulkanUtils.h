//
// Created by mikag on 3/27/2020.
//

#ifndef RELIC_VULKANUTILS_H
#define RELIC_VULKANUTILS_H

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>

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

#endif //RELIC_VULKANUTILS_H
