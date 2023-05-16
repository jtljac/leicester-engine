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
    int maxVert = 0;
    float maxDistance = glm::dot(direction, mesh->vertices.at(0).position);
    for (int i = 1; i < mesh->vertices.size(); ++i) {
        glm::vec3& vertex = mesh->vertices.at(i).position;
        float distance = glm::dot(direction, vertex);
        if (distance > maxDistance) {
            maxDistance = distance;
            maxVert = i;
        }
    }
    return mesh->vertices.at(maxVert).position;
}

Mesh* MeshCollider::getRenderMesh() {
    return this->mesh;
}

glm::mat4 MeshCollider::getRenderMeshTransform() {
    return this->getTransform();
}

