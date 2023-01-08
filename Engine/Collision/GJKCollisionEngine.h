//
// Created by jacob on 21/12/22.
//

#pragma once
#include "CollisionEngine.h"

#include <vector>
class GJKCollisionEngine : public CollisionEngine {
protected:
    /**
     * Calculate the support point in the given direction
     * @param actor1 The first actor whose collision to use to calculate the support point
     * @param actor2 The second actor whose collision to use to calculate the support point
     * @param direction The direction the support point is in
     * @return the support point
     */
    glm::vec3 getSupportPoint(Actor* actor1, Actor* actor2, glm::vec3 direction) const;

    bool testSimplex(std::vector<glm::vec3>& points, glm::vec3& direction) const;
    bool lineCase(std::vector<glm::vec3>& points, glm::vec3& direction) const;
    bool triangleCase(std::vector<glm::vec3>& points, glm::vec3& direction) const;
    bool simplexCase(std::vector<glm::vec3>& points, glm::vec3& direction) const;

    /*=================================*/
    /* EPA                             */
    /*=================================*/
    /**
     * Calculate the normals of the faces of the polytope
     * @param polytope The polytope to calculate the normals for
     * @param indices A list of indices denoting the faces of the polytope
     * @param destNormals The Vector of packed Vec4s (vec3 normal, float distance) to store the normals in
     * @return The index of the normal with the shortest distance
     */
    size_t getFaceNormals(std::vector<glm::vec3>& polytope, std::vector<size_t> indices, std::vector<glm::vec4>& destNormals) const;

    /**
     * Add the given edges to the destEdges array if their reverse doe
     * @param indices The indices the edge indexes reference
     * @param edgeIndexA The index of the first point of the edge
     * @param edgeIndexB The index of the second point of the edge
     * @param destEdges The vector to store the edge in
     */
    void addIfUniqueEdge(std::vector<size_t>& indices, size_t edgeIndexA, size_t edgeIndexB, std::vector<std::pair<size_t, size_t>>& destEdges) const;

    /**
     * Expanding Polytobe Algorithm
     * Expands the simplex until it finds the face with the normal that has the shortest penetration distance
     * @param polytope The vector containing the points of the polytube (should be the simplex)
     * @param actor1 The actor the collision is being tested for
     * @param actor2 The actor the collision is being tested against
     * @return The collision data
     */
    CollisionResult epa(std::vector<glm::vec3>& polytope, Actor* actor1, Actor* actor2) const;
public:
    /** @inherit */
    CollisionResult testCollision(Actor* actor1, Actor* actor2) override;
};
