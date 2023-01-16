//
// Created by jacob on 13/10/22.
//

#pragma once

#include <vector>

#include "Engine/Octree.h"

struct Actor;

struct Scene : public LObject {
    std::vector<Actor*> actors;
    Actor* controlledActor;
    Octree* octree = new Octree(glm::vec3(100), glm::vec3(0));

    void onCreate() override;

    void tick(double deltaTime) override;

    void onDestroy() override;

    /**
     * Add an actor to the scene
     * This sets the scene variable in the actor and runs the actor's Actor#onCreate
     * @param actor A pointer to the actor to add
     */
    // template<class ActorClass, typename std::enable_if<std::is_base_of<Actor, ActorClass>::value>::type* = nullptr>
    void addActorToScene(Actor* actor);

    void setControlledActor(Actor* actor);

    void handleInputs(int key, int scancode, int action, int mods);

    void handleMouse(double mouseX, double mouseY);
};
