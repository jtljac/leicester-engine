//
// Created by jacob on 12/10/22.
//

#include "../Renderer.h"

int Renderer::initialise(EngineSettings& settings) {
    window = glfwCreateWindow(settings.windowWidth, settings.windowWidth, settings.windowTitle.c_str(), NULL, NULL);
    if (!window) {
        return -1;
    }

    // Setup callbacks
}