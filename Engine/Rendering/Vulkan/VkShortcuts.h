//
// Created by jacob on 14/02/23.
//

#pragma once
#include <vulkan/vulkan.h>

namespace VKShortcuts {
    // Descriptor Sets
    VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags);
    VkWriteDescriptorSet createWriteDescriptorSet(uint32_t binding, VkDescriptorSet set, VkDescriptorType type, VkDescriptorBufferInfo* bufferInfo);
    VkWriteDescriptorSet createWriteDescriptorSetImage(uint32_t binding, VkDescriptorSet set, VkDescriptorType type,
                                                       VkDescriptorImageInfo* imageInfo);
}