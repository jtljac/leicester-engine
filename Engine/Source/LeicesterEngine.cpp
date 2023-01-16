//
// Created by jacob on 11/10/22.
//
#include "../LeicesterEngine.h"
#include "Utils/Logger.h"
#include <glm/gtx/string_cast.hpp>

int LeicesterEngine::initialise() {
    // Initialise GLFW
    if (!glfwInit()) return -1;

    // Initialise Renderer
    if (!renderer->initialise(this->settings)) {
        glfwTerminate();
        return -1;
    }

    glfwSetWindowUserPointer(renderer->getWindow(), this);

    // Register built in assets

    return 0;
}

Renderer* LeicesterEngine::getRenderer() const {
    return renderer;
}

void LeicesterEngine::setRenderer(Renderer* pRenderer) {
    this->renderer = pRenderer;
}

void LeicesterEngine::setCollisionEngine(CollisionEngine* pCollisionEngine) {
    this->collisionEngine = pCollisionEngine;
}

CollisionEngine* LeicesterEngine::getCollisionEngine() const {
    return collisionEngine;
}

int LeicesterEngine::startLoop() {
    double currentFrameTime;

    glfwSetKeyCallback(renderer->getWindow(), [](GLFWwindow* window, int key, int scancode, int action, int mods){
        auto* context = static_cast<LeicesterEngine*>(glfwGetWindowUserPointer(window));
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(context->renderer->getWindow(), GLFW_TRUE);
        } else {
            context->currentScene->handleInputs(key, scancode, action, mods);
        }
    });

    currentScene->onCreate();
    renderer->setupScene(*currentScene);

    // Main Loop
    while(!renderer->wantsToClose()) {
        // Get frame delta
        currentFrameTime = glfwGetTime();
        this->frameDelta = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Handle Events
        glfwPollEvents();

        // Call Ticks
        currentScene->tick(this->frameDelta);

        // Check for collision
        for (const auto& actor : currentScene->actors) {
            if (!actor->hasCollision()) continue;
            std::vector<Actor*> potentialCollisions;
            if (!collisionEngine->getNearbyColliders(actor, potentialCollisions)) continue;

            bool anyCollisions = false;
            for (const auto& otherActor: potentialCollisions) {
                if (!otherActor->hasCollision() || otherActor == actor) continue;
                CollisionResult result = collisionEngine->testCollision(actor, otherActor);
                if (actor != currentScene->controlledActor && result.collided && actor->actorCollider->collisionMode == CollisionMode::BLOCK && otherActor->actorCollider->collisionMode == CollisionMode::BLOCK) {
                    Logger::info("Normal: " + glm::to_string(result.normal) + ", Distance: " + std::to_string(result.depth));

                    // Un-collide
                    actor->setPosition(actor->getPosition() + (result.normal * -result.depth));
                }
                anyCollisions |= result.collided;
            }

            actor->actorCollider->isColliding = anyCollisions;
        }

        // Render Frame
        renderer->drawFrame(this->frameDelta, currentFrameTime, *currentScene);
    }

    currentScene->onDestroy();

    return 0;
}

void LeicesterEngine::setScene(Scene* scene) {
    if (currentScene != nullptr) delete currentScene;

    currentScene = scene;
    collisionEngine->scene = this->currentScene;
}
