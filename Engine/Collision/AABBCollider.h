//
// Created by jacob on 01/12/22.
//

#pragma once


#include "Collider.h"

struct AABBCollider : public Collider {
    float width, height, depth;

public:
    AABBCollider(float width, float height, float depth);

    glm::vec3 getBoundingBox() override;
};
