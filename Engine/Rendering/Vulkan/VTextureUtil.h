//
// Created by jacob on 27/02/23.
//

#pragma once

#include <vulkan/vulkan.h>

#include <Texture/Texture.h>

VkFormat textureFormatToVkFormat(TextureFormat textureFormat) {
    switch (textureFormat) {
        case TextureFormat::R8:
            return VK_FORMAT_R8_SRGB;
        case TextureFormat::R8G8:
            return VK_FORMAT_R8G8_SRGB;
        case TextureFormat::R8G8B8:
            return VK_FORMAT_R8G8B8_SRGB;
        case TextureFormat::R8G8B8A8:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::R16:
            return VK_FORMAT_R16_UNORM;
        case TextureFormat::R16G16:
            return VK_FORMAT_R16G16_UNORM;
        case TextureFormat::R16G16B16:
            return VK_FORMAT_R16G16B16_UNORM;
        case TextureFormat::R16G16B16A16:
            return VK_FORMAT_R16G16B16A16_UNORM;
    }
}