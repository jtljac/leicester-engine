//
// Created by jacob on 26/12/22.
//

#include "Collision/SphereCollider.h"
#include <Utils/FileUtils.h>
#include <glm/ext/matrix_transform.hpp>

static Mesh* sphereMesh = Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Shapes/Sphere.lmesh");

SphereCollider::SphereCollider(CollisionMode collisionMode, float radius) : MeshCollider(collisionMode, sphereMesh), radius(radius) {}

BoundingBox SphereCollider::getBoundingBox() {
    return {
        glm::vec3(-radius),
        glm::vec3(radius)
    };
}

glm::vec3 SphereCollider::findFurthestPointInDirection(glm::vec3 direction) const {
    return MeshCollider::findFurthestPointInDirection(direction) * radius;
}

glm::mat4 SphereCollider::getRenderMeshTransform() {
    return glm::scale(this->getTransform(), glm::vec3(radius));
}
