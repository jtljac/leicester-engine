//
// Created by jacob on 12/10/22.
//

#include "../Renderer.h"

Renderer::~Renderer() {
    if (this->window != nullptr) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
};

void Renderer::setupGLFWHints() {
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

}

int Renderer::initialiseGlfw(EngineSettings& settings) {
    glfwInit();
    setupGLFWHints();

    this->window = glfwCreateWindow(settings.windowWidth, settings.windowWidth, settings.windowTitle.c_str(), NULL, NULL);
    if (window == nullptr) {
        return -1;
    }

    glfwSetWindowUserPointer(this->window, this);

    setupGlfwCallbacks();
    return 0;
}

bool Renderer::initialise(EngineSettings& settings) {
    if (!initialiseGlfw(settings)) return false;

    return true;
}

void Renderer::setupGlfwCallbacks() {
}

bool Renderer::wantsToClose() {
    return this->window == nullptr || glfwWindowShouldClose(this->window);
}

void Renderer::cleanup() {}
