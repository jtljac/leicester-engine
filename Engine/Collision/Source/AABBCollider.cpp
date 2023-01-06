//
// Created by jacob on 01/12/22.
//

#include "Collision/AABBCollider.h"

AABBCollider::AABBCollider(const BoundingBox& boundingBox) : boundingBox(boundingBox) {}

BoundingBox AABBCollider::getBoundingBox() {
    return boundingBox;
}

glm::vec3 AABBCollider::findFurthestPointInDirection(glm::vec3 direction) const {
    return {
            (direction.x > 0 ? boundingBox.max.x : -boundingBox.min.x),
            (direction.y > 0 ? boundingBox.max.y : -boundingBox.min.y),
            (direction.z > 0 ? boundingBox.max.z : -boundingBox.min.z)
    };
}
