//
// Created by jacob on 26/12/22.
//

#pragma once


#include <glm/vec3.hpp>

struct BoundingBox {
    glm::vec3 min, max;

    BoundingBox() = default;

    BoundingBox(const glm::vec3& min, const glm::vec3& max);

    BoundingBox(const BoundingBox& boundingBox);

    BoundingBox& operator=(BoundingBox&& boundingBox) = default;
    BoundingBox& operator=(const BoundingBox& boundingBox) = default;
};
