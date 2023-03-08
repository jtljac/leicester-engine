//
// Created by jacob on 14/02/23.
//

#include "Rendering/Vulkan/VkShortcuts.h"

VkDescriptorSetLayoutBinding
VKShortcuts::createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags) {
    VkDescriptorSetLayoutBinding layoutBinding{};

    layoutBinding.binding = binding;
    layoutBinding.descriptorCount = 1;
    layoutBinding.descriptorType = type;
    layoutBinding.stageFlags = stageFlags;
    layoutBinding.pImmutableSamplers = nullptr;

    return layoutBinding;
}

VkWriteDescriptorSet VKShortcuts::createWriteDescriptorSet(uint32_t binding, VkDescriptorSet set, VkDescriptorType type,
                                                           VkDescriptorBufferInfo* bufferInfo) {
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

VkWriteDescriptorSet
VKShortcuts::createWriteDescriptorSetImage(uint32_t binding, VkDescriptorSet set, VkDescriptorType type, VkDescriptorImageInfo* imageInfo) {
    VkWriteDescriptorSet writeDescriptorSet{};

    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;

    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstSet = set;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = type;
    writeDescriptorSet.pImageInfo = imageInfo;

    return writeDescriptorSet;
}
