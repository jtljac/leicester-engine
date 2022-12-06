//
// Created by jacob on 13/10/22.
//

#include "Scene/Actor/Actor.h"

Actor::Actor(StaticMesh* mesh, Collider* collider) : actorMesh(mesh), actorCollider(collider) {

}



void Actor::onCreate() {

}


void Actor::tick(double deltaTime) {
    rotation = normalize(rotation * glm::quat({0, deltaTime, 0}));
}

void Actor::onDestroy() {

}


bool Actor::hasCollision() const {
    return this->actorCollider != nullptr;
}


bool Actor::hasMesh() const {
    return this->actorMesh != nullptr;
}

glm::vec3 Actor::getBoundingBox() const {
    if (!hasCollision()) return glm::vec3(0);

    return actorCollider->getBoundingBox();
}