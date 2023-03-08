//
// Created by jacob on 14/11/22.
//

#pragma once

#include <glm/glm.hpp>

struct GpuCameraData {
    glm::mat4 view;
    glm::mat4 proj;
};

struct GpuObjectData {
    glm::mat4 modelMatrix;
};

// For Testing, from: https://vkguide.dev/docs/chapter-4/descriptors_code_more/
struct GPUSceneData {
    glm::vec4 fogColor; // w is for exponent
    glm::vec4 fogDistances; //x for min, y for max, zw unused.
    glm::vec4 ambientColor;
    glm::vec4 sunlightDirection; //w for sun power
    glm::vec4 sunlightColor;
};