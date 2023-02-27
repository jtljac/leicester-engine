//
// Created by jacob on 13/10/22.
//

#pragma once
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct Vertex {
    glm::vec3 position, normal, colour;
    glm::vec2 uv;

    Vertex() = default;
    Vertex(glm::vec3 position, glm::vec3 normal, glm::vec3 colour, glm::vec2 uv);
};
