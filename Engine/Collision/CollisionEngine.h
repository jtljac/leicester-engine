//
// Created by jacob on 21/12/22.
//

#pragma once
#include <Scene/Actor/Actor.h>

#include <vector>

struct Scene;

struct CollisionResult {
    bool collided;
};

class CollisionEngine {
protected:
    Scene* scene;
public:
    bool getNearbyColliders(Actor* actor, std::vector<Actor*>& destPotentialActors);
    virtual CollisionResult testCollision(Actor* actor1, Actor* actor2) = 0;
};
