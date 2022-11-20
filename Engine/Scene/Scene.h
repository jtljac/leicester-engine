//
// Created by jacob on 13/10/22.
//

#pragma once

#include <vector>

#include "Actor/Actor.h"

struct Scene : public LObject {
    std::vector<Actor> actors;

    void onCreate() override;

    void tick(double deltaTime) override;

    void onDestroy() override;
};
