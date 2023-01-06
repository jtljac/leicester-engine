//
// Created by jacob on 01/12/22.
//

#pragma once


#include "Collider.h"

struct AABBCollider : public Collider {
    BoundingBox boundingBox;

public:
    AABBCollider(const BoundingBox& boundingBox);

    BoundingBox getBoundingBox() override;

    [[nodiscard]] glm::vec3 findFurthestPointInDirection(glm::vec3 direction) const override;
};
