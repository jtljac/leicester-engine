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

bool VKShortcuts::createAllocatedImage(VmaAllocator allocator, VkFormat format, VkExtent3D extent,
                                       VkImageUsageFlags imageUsage, VmaAllocationCreateFlags memoryFlags,
                                       AllocatedImage& outAllocatedImage) {
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;

    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent = extent;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = imageUsage;

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.flags = memoryFlags;
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    return vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &outAllocatedImage.image, &outAllocatedImage.allocation, nullptr) == VK_SUCCESS;
}

bool VKShortcuts::createImageView(VkDevice device, VkFormat format, VkImage image, VkImageAspectFlags aspectMask,
                                  VkImageView& outImageView) {

    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = nullptr;

    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
    };
    imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    return vkCreateImageView(device, &imageViewCreateInfo, nullptr, &outImageView) == VK_SUCCESS;
}
