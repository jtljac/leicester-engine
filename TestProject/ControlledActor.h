//
// Created by jacob on 06/01/23.
//

#pragma once


#include <Scene/Actor/Actor.h>
#include "Collision/SphereCollider.h"

class ControlledActor : public Actor {
    double lastMouseY = 0, lastMouseX = 0;
public:
    ControlledActor(StaticMesh* mesh, Collider* collider);

    void handleInput(int key, int scancode, int action, int mods) override;

    void handleMouse(double mouseX, double mouseY) override;
};
