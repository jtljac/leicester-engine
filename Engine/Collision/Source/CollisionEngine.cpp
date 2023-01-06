//
// Created by jacob on 21/12/22.
//

#include "Collision/CollisionEngine.h"
#include <Scene/Scene.h>

bool CollisionEngine::getNearbyColliders(Actor* actor, std::vector<Actor*>& destPotentialActors) {
    scene->octree->getCloseActors(actor, destPotentialActors);

    return !destPotentialActors.empty();
}
