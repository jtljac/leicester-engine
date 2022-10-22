//
// Created by jacob on 12/10/22.
//

#pragma once

// Setup Environment
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>

#include <Utils/EngineSettings.h>

#include <vector>

class Renderer {
protected:
    GLFWwindow* window = nullptr;
    virtual int initialiseGlfw(EngineSettings& settings);
    virtual void setupGLFWHints();
public:
    Renderer() = default;
    virtual ~Renderer() = 0;

    /**
     * Initialise the renderer
     */
    virtual bool initialise(EngineSettings& settings);

    /**
     * Draw a frame
     * @param deltaTime The length of the last frame in seconds
     */
    virtual void drawFrame(double deltaTime) = 0;

    virtual void cleanup();

    /**
     * Setup callbacks for GLFW Events
     */
     virtual void setupGlfwCallbacks();

    /**
     * Get if the windows wants to close
     */
    bool wantsToClose();
};
