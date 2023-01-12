//
// Created by jacob on 26/12/22.
//

#include <glm/geometric.hpp>
#include "Collision/MeshCollider.h"

MeshCollider::MeshCollider(CollisionMode collisionMode, Mesh* mesh) : Collider(collisionMode), mesh(mesh) {}

BoundingBox MeshCollider::getBoundingBox() {
    return mesh->boundingBox;
}

glm::vec3 MeshCollider::findFurthestPointInDirection(glm::vec3 direction) const {
    glm::vec3 maxPoint;
    float maxDistance = 0;
    for (const auto& vertex: mesh->vertices) {
        float distance = glm::dot(vertex.position, direction);
        if (distance > maxDistance) {
            maxDistance = distance;
            maxPoint = vertex.position;
        }
    }
    return maxPoint;
}

Mesh* MeshCollider::getRenderMesh() {
    return this->mesh;
}

glm::mat4 MeshCollider::getRenderMeshTransform() {
    return glm::mat4(1);
}

