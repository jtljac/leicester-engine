/**
 * This code (and it's source) has been created using the following tutorial:
 * https://vkguide.dev/docs/chapter-3/push_constants/     MIT Licence: https://github.com/vblanco20-1/vulkan-guide/blob/master/LICENSE.txt
 */
#pragma once

#include <glm/mat4x4.hpp>

struct MeshPushConstants {
    glm::mat4 model;
};
