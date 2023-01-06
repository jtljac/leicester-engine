//
// Created by jacob on 26/12/22.
//

#include "Collision/SphereCollider.h"
#include <Utils/FileUtils.h>
#include <glm/ext/matrix_transform.hpp>

Mesh* SphereCollider::sphereRenderMesh = Mesh::createNewMeshFromFile(FileUtils::getAssetsPath() + "/Shapes/Sphere.lmesh");

SphereCollider::SphereCollider(float radius) : radius(radius) {}

BoundingBox SphereCollider::getBoundingBox() {
    return {
        glm::vec3(radius),
        glm::vec3(radius)
    };
}

glm::vec3 SphereCollider::findFurthestPointInDirection(glm::vec3 direction) const {
    return direction * radius;
}

Mesh* SphereCollider::getRenderMesh() {
    return this->sphereRenderMesh;
}

glm::mat4 SphereCollider::getRenderMeshTransform() {
    return glm::scale(glm::mat4(1.f), glm::vec3(this->radius));
}
