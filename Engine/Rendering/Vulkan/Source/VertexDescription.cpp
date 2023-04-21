//
// Created by jacob on 30/10/22.
//

#include <Rendering/Vulkan/VertexDescription.h>

VertexDescription VertexDescription::getVertexDescription() {
    VertexDescription description;

    {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        description.bindings.push_back(bindingDescription);
    }

    // Position (Location 0)
    {
        VkVertexInputAttributeDescription posAttributeDescription = {};
        posAttributeDescription.binding = 0;
        posAttributeDescription.location = 0;
        posAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        posAttributeDescription.offset = offsetof(Vertex, position);

        description.attributes.push_back(posAttributeDescription);
    }


    // Normal (Location 1)
    {
        VkVertexInputAttributeDescription normAttributeDescription = {};
        normAttributeDescription.binding = 0;
        normAttributeDescription.location = 1;
        normAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        normAttributeDescription.offset = offsetof(Vertex, normal);

        description.attributes.push_back(normAttributeDescription);
    }

    // Tangent (Location 2)
    {
        VkVertexInputAttributeDescription tangAttributeDescription = {};
        tangAttributeDescription.binding = 0;
        tangAttributeDescription.location = 2;
        tangAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        tangAttributeDescription.offset = offsetof(Vertex, tangent);

        description.attributes.push_back(tangAttributeDescription);
    }

    // Colour (Location 3)
    {
        VkVertexInputAttributeDescription colAttributeDescription = {};
        colAttributeDescription.binding = 0;
        colAttributeDescription.location = 3;
        colAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        colAttributeDescription.offset = offsetof(Vertex, colour);

        description.attributes.push_back(colAttributeDescription);
    }

    // UV (Location 4)
    {
        VkVertexInputAttributeDescription uvAttributeDescription = {};
        uvAttributeDescription.binding = 0;
        uvAttributeDescription.location = 4;
        uvAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
        uvAttributeDescription.offset = offsetof(Vertex, uv);

        description.attributes.push_back(uvAttributeDescription);
    }

    return description;
}
