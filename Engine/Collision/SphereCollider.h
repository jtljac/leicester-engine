//
// Created by jacob on 26/12/22.
//

#pragma once


#include <glm/fwd.hpp>
#include "Collider.h"
#include "Mesh/Mesh.h"

class SphereCollider : public Collider {
protected:
    float radius;
    static Mesh* renderMesh;
public:
    SphereCollider(CollisionMode collisionMode, float radius);

    BoundingBox getBoundingBox() override;

    glm::vec3 findFurthestPointInDirection(glm::vec3 direction) const override;

    Mesh* getRenderMesh() override;

    glm::mat4 getRenderMeshTransform() override;
};
