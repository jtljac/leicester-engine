//
// Created by jacob on 11/10/22.
//
#include "../LeicesterEngine.h"
#include <Utils/EngineSettings.h>


int LeicesterEngine::initialise() {
    // Initialise GLFW
    if (!glfwInit()) return -1;

    EngineSettings settings;

    // Initialise Renderer
    if (!renderer->initialise(settings)) {
        glfwTerminate();
        return -1;
    }

    double currentFrameTime;

    // Main Loop
    while(!renderer->wantsToClose()) {
        // Get frame delta
        currentFrameTime = glfwGetTime();
        this->frameDelta = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        // Render Frame
        renderer->drawFrame(this->frameDelta, currentFrameTime);
        // Handle Events
        glfwPollEvents();
    }

    return 0;
}

void LeicesterEngine::setRenderer(Renderer* pRenderer) {
    this->renderer = pRenderer;
}