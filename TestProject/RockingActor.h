//
// Created by jacob on 04/05/23.
//

#pragma once


#include "Scene/Actor/Actor.h"

class RockingActor : public Actor {
    glm::vec3 origin;
    float distance;
    float counter = 0;
public:
    RockingActor(StaticMesh* mesh, Collider* collider, glm::vec3 origin, float distance);

private:
    void tick(double deltaTime) override;
};
