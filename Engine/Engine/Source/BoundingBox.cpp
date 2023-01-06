//
// Created by jacob on 26/12/22.
//

#include "Engine/BoundingBox.h"

BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

BoundingBox::BoundingBox(const BoundingBox& boundingBox) : min(boundingBox.min), max(boundingBox.max) {}
