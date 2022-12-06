//
// Created by jacob on 01/12/22.
//

#include "Collision/AABBCollider.h"

AABBCollider::AABBCollider(float width, float height, float depth) : width(width), height(height), depth(depth) {}

glm::vec3 AABBCollider::getBoundingBox() {
    return glm::vec3(width, height, depth);
}
