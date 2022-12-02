//
// Created by jacob on 13/10/22.
//

#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Mesh/StaticMesh.h>
#include <Collision/Collider.h>
#include "Engine/LObject.h"

struct Actor : public LObject {
    // Appearance and collision
    StaticMesh *actorMesh = nullptr;
    Collider *actorCollider = nullptr;

    // Position, rotation, scale
    glm::vec3 position{0, 0, 0};
    glm::vec3 scale{1, 1, 1};
    glm::quat rotation = glm::quat({0, 0, 0});

    Actor(StaticMesh *mesh, Collider *collider);

    void onCreate() override;

    void tick(double deltaTime) override;

    void onDestroy() override;

    // Utils
    bool hasCollision() const;

    bool hasMesh() const;

    glm::vec3 getBoundingBox() const;
};