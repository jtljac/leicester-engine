//
// Created by jacob on 01/12/22.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <algorithm>
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
    int getBoundingBoxOctant(glm::vec3 bbPosition, const BoundingBox& bb);

    /**
     * Get all the quadrants overlapped by the bounding box
     * @param bbPosition the position of the bounding box
     * @param bbSize the dimensions of the bounding box
     * @param outOctants The vector the octants will be added to
     */
    void getAllOverlappedOctants(glm::vec3 bbPosition, const BoundingBox& bb, std::vector<Octree*>& outOctants);

    /**
     * Get the octant the given point is withing
     * @param point The point to check
     * @return the index of the octant the point is within
     */
    int getPointOctant(glm::vec3 point);

    /**
     * Create the octants and spill each actor into their relevant ones
     */
    void spill();

public:

    Octree(glm::vec3 dimensions, glm::vec3 position, int depth=0) : dimensions(dimensions), position(position), depth(depth) {};
    ~Octree() {
        for (Octree* subTree : this->subTrees) {
            delete subTree;
        }
    }

    /**
     * Insert an actor into the octree
     * @param actor The actor to insert
     */
    void insertNode(Actor* actor);

    /**
     * Get all the actors close to the given actor
     * @param actor The actor being checked for
     * @param outVector The vector that the close actors will be added to
     */
    void getCloseActors(Actor* actor, std::vector<Actor*>& outVector);

    /**
     * Get all the actors in the octree
     * @param outVector
     */
    void getAllActors(std::vector<Actor*>& outVector);

    /**
     * Remove the given actor from the octree
     * This operation parses the whole octree and thus can be expensive
     * @param actor The actor to remove
     * @return true if the actor was removed
     */
    bool removeActor(Actor* actor);

    /**
     * Remove the given actor from the octree, giving it's location as of when it was inserted in the tree
     * @param actor The actor to remove
     * @param positionHint The position of the actor when it was inserted
     * @return true if the actor was removed
     */
    bool removeActor(Actor* actor, glm::vec3 positionHint);

    /**
     * Remove all actors and subtrees from this Octree
     */
    void clearTree();
};
