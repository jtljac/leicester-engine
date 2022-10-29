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

bool Renderer::initialiseGlfw(EngineSettings& settings) {
    glfwInit();
    setupGLFWHints();

    this->window = glfwCreateWindow(settings.windowWidth, settings.windowHeight, settings.windowTitle.c_str(), NULL, NULL);
    if (window == nullptr) {
        return false;
    }

    glfwSetWindowUserPointer(this->window, this);

    setupGlfwCallbacks();
    return true;
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

GLFWwindow* Renderer::getWindow() {
    return this->window;
}

