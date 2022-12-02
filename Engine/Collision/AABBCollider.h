//
// Created by jacob on 01/12/22.
//

#pragma once


#include "Collider.h"

struct AABBCollider : public Collider {
    float width, height;

public:
    AABBCollider(float width, float height);
};
