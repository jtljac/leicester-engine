/**
 * The implementations for this class have been created by following these tutorials:
 * https://blog.winter.dev/2020/gjk-algorithm/
 * https://blog.winter.dev/2020/epa-algorithm/
 * https://www.youtube.com/watch?v=ajv46BSqcK4
 *
 * Their implementations have been mixed and optimised, with edits made to make the implementation fit with this project
 * and the libraries used by this project
 */

#include <algorithm>
#include "../GJKCollisionEngine.h"
#include "Utils/Logger.h"

const static glm::vec3 ORIGIN = glm::vec3(0);

GJKState GJKCollisionEngine::testSimplex(std::vector<glm::vec3>& points, glm::vec3& direction) const {
    switch(points.size()) {
        case 2:
            return lineCase(points, direction);
        case 3:
            return triangleCase(points, direction);
        case 4:
            return simplexCase(points, direction);
    }
    // Should never get here
    return GJKState::BUILDING;
}

GJKState GJKCollisionEngine::lineCase(std::vector<glm::vec3>& points, glm::vec3& direction) const {
    glm::vec3 ab = points[0] - points[1];
    glm::vec3 a0 = - points[1];

    direction = glm::cross(glm::cross(ab, a0), ab);

    if (std::isnan(direction.x)) return GJKState::MISS;


    if (glm::dot(direction, direction) < 0.001f) {
        float min = std::abs(glm::dot(ab, {1, 0, 0}));
        direction = glm::cross(ab, {1, 0, 0});

        float newMin = std::abs(glm::dot(ab, {0, 1, 0}));
        if (newMin < min) {
            direction = glm::cross(ab, {0, 1, 0});
            min = newMin;
        }

        newMin = std::abs(glm::dot(ab, {0, 0, 1}));
        if (newMin < min) {
            direction = glm::cross(ab, {0, 0, 1});
        }
    }

    return GJKState::BUILDING;
}

GJKState GJKCollisionEngine::triangleCase(std::vector<glm::vec3>& points, glm::vec3& direction) const {
    glm::vec3 ab = points[1] - points[2];
    glm::vec3 ac = points[0] - points[2];
    glm::vec3 a0 = - points[2];

    glm::vec3 n = glm::cross(ab, ac);

    if (glm::dot(n, a0) > 0) {
        direction = n;
    } else {
        direction = -n;
    }

    return GJKState::BUILDING;
}

GJKState GJKCollisionEngine::simplexCase(std::vector<glm::vec3>& points, glm::vec3& direction) const {
    // Where a is the most recently added point
    glm::vec3 ab = points[2] - points[3];
    glm::vec3 ac = points[1] - points[3];
    glm::vec3 ad = points[0] - points[3];
    glm::vec3 a0 = - points[3];

    glm::vec3 abcn = glm::cross(ab, ac);
    glm::vec3 acdn = glm::cross(ac, ad);
    glm::vec3 adbn = glm::cross(ad, ab);

    if (glm::dot(abcn, a0) > 0) {
        points.erase(points.begin());
        return triangleCase(points, direction);
    } else if (glm::dot(acdn, a0) > 0) {
        points.erase(points.begin() + 2);
        return triangleCase(points, direction);
    } else if (glm::dot(adbn, a0) > 0) {
        points.erase(points.begin() + 1);
        return triangleCase(points, direction);
    }

    return GJKState::HIT;
}

glm::vec3 GJKCollisionEngine::getSupportPoint(Actor* actor1, Actor* actor2, glm::vec3 direction) const {
    const Collider* a = actor1->actorCollider;
    const Collider* b = actor2->actorCollider;

    glm::vec3 aPos = a->findFurthestPointInDirection(direction) + a->getPosition();
    glm::vec3 bPos = b->findFurthestPointInDirection(-direction) + b->getPosition();

    return aPos - bPos;
}

CollisionResult GJKCollisionEngine::testCollision(Actor* actor1, Actor* actor2) {
     glm::vec3 direction = actor2->getPosition() - actor1->getPosition();
    if (glm::dot(direction, direction) < 0.0001f) {
        direction = {1, 0, 0};
    }
    std::vector<glm::vec3> simplex = std::vector<glm::vec3>{
            getSupportPoint(actor1, actor2, direction)
    };

    // Return 0 if first point is very near the origin
    if (glm::dot(simplex[0], simplex[0]) < 0.001f) return CollisionResult{false};

    simplex.reserve(4);
    direction = ORIGIN - simplex[0];

    // Max iteration count to catch infinite loops
    int i = 20;
    while (i > 0) {
        glm::vec3 newPoint = getSupportPoint(actor1, actor2, direction);
        if (glm::dot(newPoint, direction) < 0) return CollisionResult{false};

        simplex.push_back(newPoint);
        GJKState state = testSimplex(simplex, direction);
        if (state == GJKState::HIT) return this->epa(simplex, actor1, actor2);
        else if (state == GJKState::MISS) return CollisionResult{false};
        --i;
    }

    return CollisionResult{false};
}

size_t GJKCollisionEngine::getFaceNormals(std::vector<glm::vec3>& polytope, std::vector<size_t> indices,
                                          std::vector<glm::vec4>& destNormals) const {
    size_t minTriangle = 0;
    float minDistance = std::numeric_limits<float>::max();

    for (size_t i = 0; i * 3 < indices.size(); ++i) {
        size_t I = i * 3;
        glm::vec3 a = polytope[indices[I  ]];
        glm::vec3 b = polytope[indices[I + 1]];
        glm::vec3 c = polytope[indices[I + 2]];

        glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        float distance = glm::dot(normal, a);
        if (distance < 0.f) {
            normal *= -1;
            distance *= -1;
        }

        destNormals.emplace_back(normal, distance);

        if (distance < minDistance) {
            minTriangle = i;
            minDistance = distance;
        }
    }
    return minTriangle;
}

void GJKCollisionEngine::addIfUniqueEdge(std::vector<size_t>& indices, size_t edgeIndexA, size_t edgeIndexB,
                                         std::vector<std::pair<size_t, size_t>>& destEdges) const {
    auto reverse = std::find(destEdges.begin(), destEdges.end(), std::make_pair(indices[edgeIndexB], indices[edgeIndexA]));

    if (reverse != destEdges.end()) destEdges.erase(reverse);
    else destEdges.emplace_back(indices[edgeIndexA], indices[edgeIndexB]);
}

CollisionResult GJKCollisionEngine::epa(std::vector<glm::vec3>& polytope, Actor* actor1, Actor* actor2) const {
    // create indices to denote the faces of the polytope
    std::vector<size_t> indices = {
            0, 1, 2,
            0, 3, 1,
            0, 2, 3,
            1, 3, 2
    };

    // Calculate the normals of the existing faces
    // Normal and distance packed into vec4 to avoid making a one-off structure
    std::vector<glm::vec4> normals;
    size_t minFace = getFaceNormals(polytope, indices, normals);

    glm::vec3 minNormal;
    float minDistance = std::numeric_limits<float>::max();

    while (minDistance == std::numeric_limits<float>::max()) {
        if (polytope.size() > 1000) {
            return {false};
        }
        minNormal = glm::vec3(normals[minFace]);
        minDistance = normals[minFace].w;

        // Calculate a new support point from the normal with the shortest distance
        glm::vec3 newPoint = getSupportPoint(actor1, actor2, minNormal);
        float sDistance = glm::dot(minNormal, newPoint);

        // If the distance isn't the same (within a margin), add the new point to the polytope and repair the faces
        if (std::abs(sDistance - minDistance) > 0.001f) {

            // Iterate through the normals and remove all the faces whose edges are in the same direction as the support point
            std::vector<std::pair<size_t, size_t>> uniqueEdges;
            for (size_t i = 0; i < normals.size(); ++i) {
                if (glm::dot(glm::vec3(normals[i]), newPoint) > 0) {
                    size_t f = i * 3;
                    addIfUniqueEdge(indices, f, f + 1, uniqueEdges);
                    addIfUniqueEdge(indices, f + 1, f + 2, uniqueEdges);
                    addIfUniqueEdge(indices, f + 2, f, uniqueEdges);

                    // Replace face with back and pop back, avoids slowdowns by shuffling the whole rest of the vector down
                    indices[f + 2] = indices.back();
                    indices.pop_back();
                    indices[f + 1] = indices.back();
                    indices.pop_back();
                    indices[f] = indices.back();
                    indices.pop_back();

                    normals[i] = normals.back();
                    normals.pop_back();

                    // Decrement so we check this index again
                    --i;
                }
            }

            // Use the unique edges to build new faces
            std::vector<size_t> newFaces;
            for (auto& edge: uniqueEdges) {
                newFaces.push_back(edge.first);
                newFaces.push_back(edge.second);
                newFaces.push_back(polytope.size());
            }
            polytope.push_back(newPoint);

            // Calculate the normals for the new faces
            std::vector<glm::vec4> newNormals;
            size_t newMinFace = getFaceNormals(polytope, newFaces, newNormals);

            // If the new normal is further, update the min face
            if (newNormals[newMinFace].w < minDistance) {
                minFace = newMinFace + normals.size();
            }
            minDistance = std::numeric_limits<float>::max();

            indices.insert(indices.end(), newFaces.begin(), newFaces.end());
            normals.insert(normals.end(), newNormals.begin(), newNormals.end());
        }
    }

    return {
        true,
        minDistance + 0.001f,
        minNormal
    };
}