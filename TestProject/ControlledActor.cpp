//
// Created by jacob on 06/01/23.
//

#include <GLFW/glfw3.h>
#include "ControlledActor.h"

void ControlledActor::handleInput(int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_W) --position.z;
    else if (key == GLFW_KEY_S) ++position.z;
    if (key == GLFW_KEY_A) --position.x;
    else if (key == GLFW_KEY_D) ++position.x;
    if (key == GLFW_KEY_SPACE) --position.y;
    else if (key == GLFW_KEY_LEFT_CONTROL) ++position.y;
}

ControlledActor::ControlledActor(StaticMesh* mesh, Collider* collider) : Actor(mesh, collider) {

}
