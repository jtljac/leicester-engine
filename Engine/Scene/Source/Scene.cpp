//
// Created by jacob on 13/10/22.
//

#include "Scene/Scene.h"

void Scene::onCreate() {
    for (auto& actor : actors) {
        actor.onCreate();
    }
}

void Scene::tick(double deltaTime) {
    for (auto& actor : actors) {
        actor.tick(deltaTime);
    }
}

void Scene::onDestroy() {
    for (auto& actor : actors) {
        actor.onDestroy();
    }
}
