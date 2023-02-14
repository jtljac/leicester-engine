//
// Created by jacob on 14/02/23.
//

#pragma once
#include <vulkan/vulkan.h>

namespace VKShortcuts {
    VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags) {
        VkDescriptorSetLayoutBinding layoutBinding{};

        layoutBinding.binding = binding;
        layoutBinding.descriptorCount = 1;
        layoutBinding.descriptorType = type;
        layoutBinding.stageFlags = stageFlags;
        layoutBinding.pImmutableSamplers = nullptr;

        return layoutBinding;
    }

    VkWriteDescriptorSet createWriteDescriptorSet(uint32_t binding, VkDescriptorSet set, VkDescriptorType type, VkDescriptorBufferInfo* bufferInfo) {
        VkWriteDescriptorSet writeDescriptorSet{};

        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.pNext = nullptr;

        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.dstSet = set;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = type;
        writeDescriptorSet.pBufferInfo = bufferInfo;

        return writeDescriptorSet;
    }
}