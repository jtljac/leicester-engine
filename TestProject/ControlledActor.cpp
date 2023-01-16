//
// Created by jacob on 06/01/23.
//

#include <GLFW/glfw3.h>
#include <glm/gtx/string_cast.hpp>
#include "ControlledActor.h"
#include "Utils/Logger.h"
#include <cmath>

void ControlledActor::handleInput(int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_W) position.z -= .5f;
    else if (key == GLFW_KEY_S) position.z += .5f;
    if (key == GLFW_KEY_A) position.x -= .5f;
    else if (key == GLFW_KEY_D) position.x += .5f;
    if (key == GLFW_KEY_SPACE) position.y -= .5f;
    else if (key == GLFW_KEY_LEFT_CONTROL) position.y += .5f;
}

void ControlledActor::handleMouse(double mouseX, double mouseY) {
    Logger::info("x:" + std::to_string(mouseX) + ", y:" + std::to_string(mouseY));
    float deltaX = mouseX - this->lastMouseX;
    float deltaY = this->lastMouseY - mouseY;
    lastMouseX = mouseX;
    lastMouseY = mouseY;

    glm::vec3 eulerAngles = glm::eulerAngles(rotation);
    eulerAngles.x += deltaY * .01; // .y for yaw, .z for roll
    eulerAngles.y += deltaX * .01;
    rotation = glm::quat(eulerAngles);
}


ControlledActor::ControlledActor(StaticMesh* mesh, Collider* collider) : Actor(mesh, collider) {

}
