//
// Created by jacob on 01/12/22.
//

#pragma once


#include "Collider.h"

class AABBCollider : public Collider {
    static Mesh* renderMesh;
    BoundingBox boundingBox;
public:

    AABBCollider(CollisionMode collisionMode, const BoundingBox& boundingBox);
    AABBCollider(CollisionMode collisionMode, glm::vec3 min, glm::vec3 max);

    BoundingBox getBoundingBox() override;

    [[nodiscard]] glm::vec3 findFurthestPointInDirection(glm::vec3 direction) const override;

    Mesh* getRenderMesh() override;

    glm::mat4 getRenderMeshTransform() override;
};
