//
// Created by jacob on 13/10/22.
//

#include "Scene/Actor/Actor.h"



void Actor::onCreate() {

}


void Actor::tick(float delta) {

}

void Actor::onDestroy() {

}


bool Actor::hasCollision() {
    return this->actorCollider != nullptr;
}


bool Actor::hasMesh() {
    return this->actorMesh != nullptr;
}