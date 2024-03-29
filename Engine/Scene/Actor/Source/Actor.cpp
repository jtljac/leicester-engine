//
// Created by jacob on 13/10/22.
//

#include <Scene/Actor/Actor.h>
#include <Scene/Scene.h>

Actor::Actor(StaticMesh* mesh, Collider* collider) : actorMesh(mesh), actorCollider(collider), LObject() {
    if (mesh != nullptr) mesh->setParent(this);
    if (collider != nullptr) collider->setParent(this);
}

Actor::Actor(StaticMesh* mesh, Collider* collider, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation) : actorMesh(mesh), actorCollider(collider), LObject(position, scale, rotation,
                                                                                                                                                                                     nullptr, true) {}

Actor::Actor(const Actor& otherActor) : actorMesh(otherActor.actorMesh), actorCollider(otherActor.actorCollider),
                                        LObject((LObject&) otherActor){}


void Actor::onCreate() {
    if (actorMesh != nullptr) actorMesh->onCreate();
    if (actorCollider != nullptr) actorCollider->onCreate();
}


void Actor::tick(double deltaTime) {
    if (actorMesh != nullptr) actorMesh->tick(deltaTime);
    if (actorCollider != nullptr) actorCollider->tick(deltaTime);

//    rotation = normalize(rotation * glm::quat({0, deltaTime, 0}));
}

void Actor::onDestroy() {
    if (actorMesh != nullptr) actorMesh->onDestroy();
    if (actorCollider != nullptr) actorCollider->onDestroy();
}


bool Actor::hasCollision() const {
    return this->actorCollider != nullptr;
}


bool Actor::hasMesh() const {
    return this->actorMesh != nullptr;
}

BoundingBox Actor::getBoundingBox() const {
    if (!hasCollision()) return {glm::vec3(0), glm::vec3(0)};

    return actorCollider->getBoundingBox();
}

void Actor::handleInput(int key, int scancode, int action, int mods) {}

void Actor::handleMouse(double mouseX, double mouseY) {}
