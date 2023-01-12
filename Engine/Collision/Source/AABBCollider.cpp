//
// Created by jacob on 01/12/22.
//

#include <glm/ext/matrix_transform.hpp>
#include "Collision/AABBCollider.h"
#include "Utils/FileUtils.h"

Mesh* AABBCollider::renderMesh = Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Shapes/Cube.lmesh");

AABBCollider::AABBCollider(CollisionMode collisionMode, const BoundingBox& boundingBox) : Collider(collisionMode), boundingBox(boundingBox) {}



AABBCollider::AABBCollider(CollisionMode collisionMode, const glm::vec3 min, const glm::vec3 max) : Collider(collisionMode), boundingBox(min, max) {}

BoundingBox AABBCollider::getBoundingBox() {
    return boundingBox;
}

glm::vec3 AABBCollider::findFurthestPointInDirection(glm::vec3 direction) const {
    return {
            (direction.x > 0 ? boundingBox.max.x : boundingBox.min.x),
            (direction.y > 0 ? boundingBox.max.y : boundingBox.min.y),
            (direction.z > 0 ? boundingBox.max.z : boundingBox.min.z)
    };
}

Mesh* AABBCollider::getRenderMesh() {
    return AABBCollider::renderMesh;
}

glm::mat4 AABBCollider::getRenderMeshTransform() {
    glm::vec3 scale = glm::abs(this->boundingBox.min) + glm::abs(this->boundingBox.max);
    glm::vec3 position = (this->boundingBox.min + this->boundingBox.max);
    position /= 2;
    return glm::scale(glm::translate(glm::mat4(1.f), position), scale);
}
