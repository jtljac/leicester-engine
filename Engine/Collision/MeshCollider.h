//
// Created by jacob on 26/12/22.
//

#pragma once


#include "Collider.h"
#include "Mesh/Mesh.h"
#include "Mesh/StaticMesh.h"

class MeshCollider : public Collider {
    Mesh* mesh;

public:
    explicit MeshCollider(CollisionMode collisionMode, Mesh* mesh);

    BoundingBox getBoundingBox() override;

    glm::vec3 findFurthestPointInDirection(glm::vec3 direction) const override;

    Mesh* getRenderMesh() override;

    glm::mat4 getRenderMeshTransform() override;
};
