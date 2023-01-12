//
// Created by jacob on 26/12/22.
//

#include "Collision/SphereCollider.h"
#include <Utils/FileUtils.h>
#include <glm/ext/matrix_transform.hpp>

Mesh* SphereCollider::renderMesh = Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Shapes/Sphere.lmesh");

SphereCollider::SphereCollider(CollisionMode collisionMode, float radius) : Collider(collisionMode), radius(radius) {}

BoundingBox SphereCollider::getBoundingBox() {
    return {
        glm::vec3(-radius),
        glm::vec3(radius)
    };
}

glm::vec3 SphereCollider::findFurthestPointInDirection(glm::vec3 direction) const {
    return glm::normalize(direction) * radius;
}

Mesh* SphereCollider::getRenderMesh() {
    return SphereCollider::renderMesh;
}

glm::mat4 SphereCollider::getRenderMeshTransform() {
    return glm::scale(glm::mat4(1.f), glm::vec3(this->radius));
}
