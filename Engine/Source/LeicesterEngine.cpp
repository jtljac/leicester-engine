//
// Created by jacob on 11/10/22.
//
#include "../LeicesterEngine.h"

int LeicesterEngine::initialise() {
    // Initialise GLFW
    if (!glfwInit()) return -1;

    // Initialise Renderer
    if (!renderer->initialise(this->settings)) {
        glfwTerminate();
        return -1;
    }

    return 0;
}

Renderer* LeicesterEngine::getRenderer() const {
    return renderer;
}

void LeicesterEngine::setRenderer(Renderer* pRenderer) {
    this->renderer = pRenderer;
}

int LeicesterEngine::startLoop() {
    double currentFrameTime;

    glfwSetKeyCallback(renderer->getWindow(), [](GLFWwindow* window, int key, int scancode, int action, int mods){
        auto* context = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            context->shader = (context->shader + 1) % 2;
        } else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(context->getWindow(), GLFW_TRUE);
        }
    });

    currentScene->onCreate();
    renderer->setupScene(*currentScene);

    // Main Loop
    while(!renderer->wantsToClose()) {
        // Get frame delta
        currentFrameTime = glfwGetTime();
        this->frameDelta = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Handle Events
        glfwPollEvents();

        // Call Ticks
        currentScene->tick(this->frameDelta);

        // Render Frame
        renderer->drawFrame(this->frameDelta, currentFrameTime, *currentScene);
    }

    currentScene->onDestroy();

    return 0;
}
