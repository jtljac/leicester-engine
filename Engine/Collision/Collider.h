//
// Created by jacob on 15/10/22.
//

#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <Engine/BoundingBox.h>
#include "Mesh/Mesh.h"

struct Collider {
    virtual BoundingBox getBoundingBox() = 0;
    virtual Mesh* getRenderMesh() = 0;
    virtual glm::mat4 getRenderMeshTransform() = 0;

    bool isColliding = false;

    [[nodiscard]] virtual glm::vec3 findFurthestPointInDirection(glm::vec3 direction) const = 0;
};
