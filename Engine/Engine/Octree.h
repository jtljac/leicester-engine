//
// Created by jacob on 01/12/22.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Scene/Actor/Actor.h"

class Octree {
    int MAX_DEPTH = 8;
    std::vector<Actor*> entities;
    std::vector<Octree*> subTrees;
    int depth;
    glm::vec3 position;
    glm::vec3 dimensions;

protected:
    int TOPFRONTRIGHTINDEX = 0;
    int TOPFRONTLEFINDEX = 1;
    int BOTTOMFRONTRIGHTINDEX = 2;
    int BOTTOMFRONTLEFINDEX = 3;
    int TOPBACKRIGHTINDEX = 4;
    int TOPBACKLEFTINDEX = 5;
    int BOTTOMBACKRIGHTINDEX = 6;
    int BOTTOMBACKLEFTINDEX = 7;

    /**
     * Calculate the octant the actor is in
     * @param actor
     * @return the index of the octant if the actor's bounding box is contained wholly in
     */
    int getActorOctant(Actor* actor) {
        glm::vec3 actorPosition = position - actor->position;

        // Half bb so it can be conveniently added to positions
        glm::vec3 actorBB = actor->getBoundingBox() * .5f;
        int index = 0;
        if ((actorPosition.x - actorBB.x) < 0) {
            if (actorPosition.x + actorBB.x > 0) return -1;
            index += 1;
        }

        if ((actorPosition.y - actorBB.y) < 0) {
            if (actorPosition.y + actorBB.y > 0) return -1;
            index += 2;
        }

        if ((actorPosition.z - actorBB.z) < 0) {
            if (actorPosition.z + actorBB.z > 0) return -1;
            index += 4;
        }

        return index;
    }

public:

    Octree(glm::vec3 dimensions, glm::vec3 position, int depth=0) : dimensions(dimensions), position(position), depth(depth) {};
    ~Octree() {
        for (Octree* subTree : this->subTrees) {
            delete subTree;
        }
    }

    void spill() {
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

    void insertNode(Actor* actor) {
        if (subTrees.empty()) {
            entities.push_back(actor);
            if (subTrees.size() == 8 && this->depth != this->MAX_DEPTH) spill();
        } else {
            int quadrantIndex = getActorOctant(actor);
            if (quadrantIndex < 0) {
                entities.push_back(actor);
            } else {
                subTrees[quadrantIndex]->insertNode(actor);
            }
        }
    }
};
