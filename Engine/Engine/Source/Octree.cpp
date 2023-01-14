//
// Created by jacob on 01/12/22.
//

#include "Engine/Octree.h"

int Octree::getBoundingBoxOctant(glm::vec3 bbPosition, const BoundingBox& bb) {
    glm::vec3 position = bbPosition - this->position;

    int index = 0;
    if ((position.x + bb.min.x) <= 0) {
        if (position.x + bb.max.x >= 0) return -1;
        index += 1;
    }

    if ((position.y + bb.min.y) <= 0) {
        if (position.y + bb.max.y >= 0) return -1;
        index += 2;
    }

    if ((position.z + bb.min.z) <= 0) {
        if (position.z + bb.max.z >= 0) return -1;
        index += 4;
    }

    return index;
}

void Octree::getAllOverlappedOctants(glm::vec3 bbPosition, const BoundingBox& bb, std::vector<Octree*>& outOctants) {
    glm::vec3 position = bbPosition - this->position;

    if ((position.x + bb.max.x) >= 0) {
        if ((position.y + bb.max.y) >= 0) {
            if ((position.z + bb.max.z) >= 0) {
                outOctants.push_back(subTrees[BOTTOMFRONTRIGHTINDEX]);
            }
            if (position.z + bb.min.z < 0) {
                outOctants.push_back(subTrees[BOTTOMBACKRIGHTINDEX]);
            }
        }
        if (position.y + bb.min.y < 0) {
            if ((position.z + bb.max.z) >= 0) {
                outOctants.push_back(subTrees[TOPFRONTRIGHTINDEX]);
            }
            if (position.z + bb.min.z < 0) {
                outOctants.push_back(subTrees[TOPBACKRIGHTINDEX]);
            }
        }
    }
    if (position.x + bb.min.x < 0) {
        if ((position.y + bb.max.y) >= 0) {
            if ((position.z + bb.max.z) >= 0) {
                outOctants.push_back(subTrees[BOTTOMFRONTLEFINDEX]);
            }
            if (position.z + bb.min.z < 0) {
                outOctants.push_back(subTrees[BOTTOMBACKLEFTINDEX]);
            }
        }
        if (position.y + bb.min.y < 0) {
            if ((position.z + bb.max.z) >= 0) {
                outOctants.push_back(subTrees[TOPFRONTLEFINDEX]);
            }
            if (position.z + bb.min.z < 0) {
                outOctants.push_back(subTrees[TOPBACKLEFTINDEX]);
            }
        }
    }
}

int Octree::getPointOctant(glm::vec3 point) {
    glm::vec3 position = point - this->position;

    int index = 0;
    if (position.x < 0) {
        index += 1;
    }

    if (position.y < 0) {
        index += 2;
    }

    if (position.z < 0) {
        index += 4;
    }

    return index;
}

void Octree::spill() {
    glm::vec3 newDimensions = dimensions * .5f;
    subTrees.push_back(new Octree(newDimensions, position + glm::vec3( newDimensions.x, -newDimensions.y,  newDimensions.z), depth + 1));
    subTrees.push_back(new Octree(newDimensions, position + glm::vec3(-newDimensions.x, -newDimensions.y,  newDimensions.z), depth + 1));
    subTrees.push_back(new Octree(newDimensions, position + glm::vec3( newDimensions.x,  newDimensions.y,  newDimensions.z), depth + 1));
    subTrees.push_back(new Octree(newDimensions, position + glm::vec3(-newDimensions.x,  newDimensions.y,  newDimensions.z), depth + 1));
    subTrees.push_back(new Octree(newDimensions, position + glm::vec3( newDimensions.x, -newDimensions.y, -newDimensions.z), depth + 1));
    subTrees.push_back(new Octree(newDimensions, position + glm::vec3(-newDimensions.x, -newDimensions.y, -newDimensions.z), depth + 1));
    subTrees.push_back(new Octree(newDimensions, position + glm::vec3( newDimensions.x,  newDimensions.y, -newDimensions.z), depth + 1));
    subTrees.push_back(new Octree(newDimensions, position + glm::vec3(-newDimensions.x,  newDimensions.y, -newDimensions.z), depth + 1));

    std::vector<Actor*> temp(this->entities);
    entities.clear();
    for (Actor* actor : temp) {
        insertNode(actor);
    }
}

void Octree::insertNode(Actor* actor) {
    if (subTrees.empty()) {
        entities.push_back(actor);
        if (entities.size() == 8 && this->depth != this->MAX_DEPTH) spill();
    } else {
        int quadrantIndex = getBoundingBoxOctant(actor->getPosition(), actor->getBoundingBox());
        if (quadrantIndex < 0) {
            entities.push_back(actor);
        } else {
            subTrees[quadrantIndex]->insertNode(actor);
        }
    }
}

void Octree::getCloseActors(Actor* actor, std::vector<Actor*>& outVector) {
    for (const auto& item: entities) {
        outVector.push_back(item);
    }

    if (!subTrees.empty()) {
        std::vector<Octree*> overlappedOctants;
        getAllOverlappedOctants(actor->getPosition(), actor->getBoundingBox(), overlappedOctants);
        for (const auto& octant: overlappedOctants) {
            octant->getCloseActors(actor, outVector);
        }
    }
}

void Octree::getAllActors(std::vector<Actor*>& outVector) {
    for (const auto& entity: entities) {
        outVector.push_back(entity);
    }

    for (const auto& subTree: subTrees) {
        subTree->getAllActors(outVector);
    }
}

bool Octree::removeActor(Actor* actor) {
    auto index = std::find(entities.begin(), entities.end(), actor);
    if (index != entities.end()) {
        for (const auto& subTree: subTrees) {
            if (removeActor(actor)) return true;
        }
        return false;
    }

    entities.erase(index);
    return true;
}

bool Octree::removeActor(Actor* actor, glm::vec3 positionHint) {
    auto index = std::find(entities.begin(), entities.end(), actor);
    if (index != entities.end()) {
        if (!subTrees.empty()) {
            return subTrees[getPointOctant(positionHint)]->removeActor(actor, positionHint);
        }
        return false;
    }

    entities.erase(index);
    return true;
}

void Octree::clearTree() {
    entities.clear();
    for (auto& subTree: subTrees) {
        subTree->clearTree();
        delete subTree;
    }
    subTrees.clear();
}
