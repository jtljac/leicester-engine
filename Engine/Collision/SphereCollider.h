//
// Created by jacob on 26/12/22.
//

#pragma once


#include <glm/fwd.hpp>
#include "MeshCollider.h"

class SphereCollider : public MeshCollider {
protected:
    float radius;
public:
    SphereCollider(CollisionMode collisionMode, float radius);

    BoundingBox getBoundingBox() override;

    glm::vec3 findFurthestPointInDirection(glm::vec3 direction) const override;

    glm::mat4 getRenderMeshTransform() override;
};
