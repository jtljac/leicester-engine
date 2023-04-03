/**
 * This file (and it's sources) has been created by following this tutorial:
 * https://vkguide.dev/docs/chapter-4/double_buffering/     MIT Licence: https://github.com/vblanco20-1/vulkan-guide/blob/master/LICENSE.txt
 *
 * The guide's example has been modified
 */
#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "AllocationStructures.h"

struct FrameData {
    // GPU Synchronisation
    VkSemaphore presentSemaphore, renderSemaphore;
    VkFence renderFence;

    // Commands
    VkCommandPool commandPool;
    VkCommandBuffer deferredCommandBuffer;
    VkCommandBuffer combinationCommandBuffer;

    // Descriptor Sets
    VkDescriptorSet globalDescriptor;
    VkDescriptorSet deferredPassDescriptor;
    VkDescriptorSet combinationPassDescriptor;

    // Buffers
    AllocatedBuffer cameraBuffer;
    AllocatedBuffer objectBuffer;
};

struct SwapchainData {
    // Swapchain
    VkImage swapchainImage;
    VkImageView swapchainImageView;

    // Depth Buffer
    AllocatedImage depthImage;
    VkImageView depthImageView;

    VkFramebuffer framebuffer;
};

struct GBufferData {
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    AllocatedImage image{};
    VkImageView imageView = VK_NULL_HANDLE;
};

struct DeferredFrameData {
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    GBufferData position{}, albedo{}, metallicRoughnessAO{}, normal{};
    VkSemaphore deferredSemaphore = VK_NULL_HANDLE;
};