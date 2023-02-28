//
// Created by jacob on 27/02/23.
//

#pragma once

#include <vulkan/vulkan.h>

#include <Texture/Texture.h>
#include "AllocationStructures.h"

VkFormat textureFormatToVkFormat(TextureFormat textureFormat);

struct VTexture {
    AllocatedImage image{};
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;

    VTexture() = default;

    VTexture(const AllocatedImage& image, VkImageView imageView, VkSampler sampler);
};