//
// Created by jacob on 21/12/22.
//

#include "../GJKCollisionEngine.h"

const static glm::vec3 ORIGIN = glm::vec3(0);

bool GJKCollisionEngine::testSimplex(std::vector<glm::vec3>& points, glm::vec3& direction) const {
    switch(points.size()) {
        case 2:
            return lineCase(points, direction);
        case 3:
            return triangleCase(points, direction);
        case 4:
            return simplexCase(points, direction);
    }
    // Should never get here
    return false;
}

bool GJKCollisionEngine::lineCase(std::vector<glm::vec3>& points, glm::vec3& direction) const {
    glm::vec3 ab = points[1] - points[0];
    glm::vec3 a0 = - points[0];

    direction = glm::cross(glm::cross(ab, a0), ab);

    return false;
}

bool GJKCollisionEngine::triangleCase(std::vector<glm::vec3>& points, glm::vec3& direction) const {
    glm::vec3 ab = points[1] - points[0];
    glm::vec3 ac = points[2] - points[0];
    glm::vec3 a0 = - points[0];

    glm::vec3 n = glm::cross(ab, ac);

    if (glm::dot(n, a0) > 0) {
        direction = n;
    } else {
        direction = -n;
    }

    return false;
}

bool GJKCollisionEngine::simplexCase(std::vector<glm::vec3>& points, glm::vec3& direction) const {
    // Where d is the most recently added point
    glm::vec3 ab = points[1] - points[0];
    glm::vec3 ac = points[2] - points[0];
    glm::vec3 ad = points[3] - points[0];
    glm::vec3 a0 = - points[0];

    glm::vec3 abcn = glm::cross(ab, ac);
    glm::vec3 acdn = glm::cross(ac, ad);
    glm::vec3 adbn = glm::cross(ad, ab);

    if (glm::dot(abcn, a0) > 0) {
        points.erase(points.begin() + 3);
        return triangleCase(points, direction);
    } else if (glm::dot(acdn, a0) > 0) {
        points.erase(points.begin() + 1);
        return triangleCase(points, direction);
    } else if (glm::dot(adbn, a0) > 0) {
        points.erase(points.begin() + 2);
        return triangleCase(points, direction);
    }

    return true;
}

CollisionResult GJKCollisionEngine::testCollision(Actor* actor1, Actor* actor2) {
    const Collider* a = actor1->actorCollider;
    const Collider* b = actor2->actorCollider;

    glm::vec3 direction = glm::normalize(actor2->position - actor1->position);
    std::vector<glm::vec3> simplex = std::vector<glm::vec3>{
            (a->findFurthestPointInDirection(direction) + actor1->position) - (b->findFurthestPointInDirection(-direction) + actor2->position)
    };
    simplex.reserve(4);
    direction = ORIGIN - simplex[0];

    while (true) {
        glm::vec3 newPoint = (a->findFurthestPointInDirection(direction) + actor1->position) - (b->findFurthestPointInDirection(-direction) + actor2->position);
        if (glm::dot(newPoint, direction) < 0) return CollisionResult{false};

        simplex.push_back(newPoint);
        if (testSimplex(simplex, direction)) return CollisionResult{true};
    }

    // Shouldn't ever get here
    return {false};
}