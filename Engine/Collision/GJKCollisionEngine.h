//
// Created by jacob on 21/12/22.
//

#pragma once
#include "CollisionEngine.h"

#include <vector>
class GJKCollisionEngine : public CollisionEngine {
private:
    bool testSimplex(std::vector<glm::vec3>& points, glm::vec3& direction) const;
    bool lineCase(std::vector<glm::vec3>& points, glm::vec3& direction) const;
    bool triangleCase(std::vector<glm::vec3>& points, glm::vec3& direction) const;
    bool simplexCase(std::vector<glm::vec3>& points, glm::vec3& direction) const;
public:
    CollisionResult testCollision(Actor* actor1, Actor* actor2) override;
};
