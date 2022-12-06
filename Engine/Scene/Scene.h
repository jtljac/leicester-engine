//
// Created by jacob on 13/10/22.
//

#pragma once

#include <vector>

#include "Actor/Actor.h"
#include "Engine/Octree.h"

struct Scene : public LObject {
    std::vector<Actor> actors;
    Octree* octree = new Octree(glm::vec3(100), glm::vec3(0));

    void onCreate() override;

    void tick(double deltaTime) override;

    void onDestroy() override;

    // TODO: Add actor to scene
};
