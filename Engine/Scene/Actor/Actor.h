//
// Created by jacob on 13/10/22.
//

#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Mesh/StaticMesh.h>
#include <Collision/Collider.h>
#include "Engine/LObject.h"

struct Scene;

struct Actor : public LObject {

// Appearance and collision
    StaticMesh *actorMesh = nullptr;
    Collider *actorCollider = nullptr;

    Scene* scene = nullptr;

    Actor(StaticMesh *mesh, Collider *collider);
    Actor(StaticMesh* mesh, Collider* collider, const glm::vec3& position, const glm::vec3& scale,
          const glm::quat& rotation);
    Actor(const Actor& otherActor);

    void onCreate() override;

    void tick(double deltaTime) override;

    void onDestroy() override;

    // Utils
    [[nodiscard]] bool hasCollision() const;

    [[nodiscard]] bool hasMesh() const;

    [[nodiscard]] BoundingBox getBoundingBox() const;

    virtual void handleInput(int key, int scancode, int action, int mods);

    virtual void handleMouse(double mouseX, double mouseY);
};