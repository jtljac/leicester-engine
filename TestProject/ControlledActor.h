//
// Created by jacob on 06/01/23.
//

#pragma once


#include <Scene/Actor/Actor.h>
#include "Collision/SphereCollider.h"

class ControlledActor : public Actor {
public:
    ControlledActor(StaticMesh* mesh, Collider* collider);

    void handleInput(int key, int scancode, int action, int mods) override;
};
