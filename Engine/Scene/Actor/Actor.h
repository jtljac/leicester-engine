//
// Created by jacob on 13/10/22.
//

#pragma once
#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"

#include <Mesh/Mesh.h>
#include <Collision/Collider.h>

struct Actor {
    // Appearance and collision
    Mesh* actorMesh = nullptr;
    Collider* actorCollider = nullptr;

    // Position, rotation, scale
    glm::vec3 position, scale;
    glm::quat rotation;

    void onCreate();
    void tick(float delta);
    void onDestroy();

    // Utils
    bool hasCollision();
    bool hasMesh();
};
