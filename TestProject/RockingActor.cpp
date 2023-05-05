//
// Created by jacob on 04/05/23.
//

#include "RockingActor.h"
#include <glm/gtx/string_cast.hpp>

void RockingActor::tick(double deltaTime) {
    Actor::tick(deltaTime);
    counter += deltaTime;
    glm::vec3 newPos = origin + glm::vec3(sin(counter / 2) * distance, 0, 0);
    this->setLocalPosition(newPos);

    Logger::info(std::to_string(deltaTime));
}

RockingActor::RockingActor(StaticMesh* mesh, Collider* collider, glm::vec3 origin, const float distance)
        : Actor(mesh, collider),
          origin(origin),
          distance(distance) {}
