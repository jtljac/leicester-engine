//
// Created by jacob on 13/10/22.
//

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
    }
}

void Scene::addActorToScene(Actor* actor) {
    this->actors.push_back(actor);
    this->octree->insertNode(actor);
    actor->scene = this;
    actor->onCreate();
}

