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