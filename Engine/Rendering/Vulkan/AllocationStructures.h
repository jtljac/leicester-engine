/**
 * This code was created by following the following guide:
 * https://vkguide.dev/docs/chapter-3/triangle_mesh_code/   MIT Licence: https://github.com/vblanco20-1/vulkan-guide/blob/master/LICENSE.txt
 */
#pragma once

#include <vk_mem_alloc.h>

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct AllocatedImage {
    VkImage image;
    VmaAllocation allocation;
};