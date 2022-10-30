/**
 * This code (and it's source) has been created using the following tutorial:
 * https://vkguide.dev/docs/chapter-3/triangle_mesh_code/   MIT Licence: https://github.com/vblanco20-1/vulkan-guide/blob/master/LICENSE.txt
 */

#pragma once

#include <vector>

#include <Mesh/Vertex.h>

#include <vulkan/vulkan.h>

struct VertexDescription {
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    VkPipelineVertexInputStateCreateFlags flags = 0;

    static VertexDescription getVertexDescription();
};
