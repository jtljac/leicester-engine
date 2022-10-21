//
// Created by jacob on 13/10/22.
//

#pragma once
#include <glm/vec3.hpp>

struct Vertex {
    glm::vec3 position, normal, tangent;

    Vertex() = default;
    Vertex(glm::vec3 position, glm::vec3 normal, glm::vec3 tangent);
};
