//
// Created by jacob on 12/10/22.
//

#include "../Renderer.h"



void Renderer::setupGLFWHints() {}

int Renderer::initialiseGlfw(EngineSettings& settings) {
    glfwInit();
    setupGLFWHints();

    this->window = glfwCreateWindow(settings.windowWidth, settings.windowWidth, settings.windowTitle.c_str(), NULL, NULL);
    if (!window) {
        return -1;
    }

    glfwSetWindowUserPointer(this->window, this);

    setupGlfwCallbacks();
}

int Renderer::initialise(EngineSettings& settings) {
    initialiseGlfw(settings);

}

void Renderer::setupGlfwCallbacks() {
    glfwSetFramebufferSizeCallback(window, )
}

bool Renderer::wantsToClose() {
    return glfwWindowShouldClose(window);
}
