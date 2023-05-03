//
// Created by jacob on 13/10/22.
//

#include <GLFW/glfw3.h>
#include "Scene/Scene.h"
#include "../Actor/Actor.h"

void Scene::onCreate() {
    for (Actor* actor : actors) {
        actor->onCreate();
    }
}

void Scene::tick(double deltaTime) {
    octree->clearTree();
    for (Actor* actor: actors) {
        octree->insertNode(actor);
    }

    for (auto& actor : actors) {
        actor->tick(deltaTime);
    }
}

void Scene::onDestroy() {
    for (auto& actor : actors) {
        actor->onDestroy();
        delete actor;
    }
}

void Scene::addActorToScene(Actor* actor) {
    this->actors.push_back(actor);
    this->octree->insertNode(actor);
    actor->scene = this;
    actor->onCreate();
}

void Scene::setControlledActor(Actor* actor) {
    this->controlledActor = actor;
}

void Scene::handleInputs(int key, int scancode, int action, int mods) {
    if (controlledActor != nullptr) {
        controlledActor->handleInput(key, scancode, action, mods);
    }
}

void Scene::handleMouse(double mouseX, double mouseY) {
    if (controlledActor != nullptr) {
        controlledActor->handleMouse(mouseX, mouseY);
    }
}

