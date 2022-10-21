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
    GLFWwindow* window;
protected:
    virtual int initialiseGlfw(EngineSettings& settings);
    virtual void setupGLFWHints();

    virtual void glfwResizeWindow(GLFWwindow* window, int width, int height) = 0;
public:
    virtual ~Renderer() = 0;

    /**
     * Initialise the renderer
     */
    virtual int initialise(EngineSettings& settings);

    /**
     * Setup callbacks for GLFW Events
     */
     virtual void setupGlfwCallbacks();

    /**
     * Get if the windows wants to close
     */
    bool wantsToClose();

    virtual void drawFrame(double deltaTime) = 0;
};
