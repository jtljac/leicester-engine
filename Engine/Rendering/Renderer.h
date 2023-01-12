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
#include "Scene/Scene.h"

class Renderer {
protected:
    EngineSettings* settings = nullptr;
    GLFWwindow* window = nullptr;

    /**
     * Initialise glfw settings
     * @param settings the engine settings
     * @return
     */
    virtual bool initialiseGlfw(EngineSettings& settings);

    /**
     * Set glfw hits
     */
    virtual void setupGLFWHints();
public:
    Renderer() = default;
    virtual ~Renderer() = 0;

    /**
     * Initialise the renderer
     * @param settings the engine settings
     */
    virtual bool initialise(EngineSettings& settings);

    /**
     * Initialise the shader pipeline
     * @param settings the engine settings
     * @return true if successful
     */
    virtual void setupScene(Scene& scene) = 0;

    /**
     * Register a mesh in the renderer
     * Sets the vertices and indices ID
     * @param mesh The mesh to register
     * @return true if successful, false if the mesh has already been registered
     */
    virtual bool registerMesh(Mesh* mesh) = 0;

    /**
     * Register a material in the renderer
     * Sets the material ID
     * @param material The material to register
     * @return true if successful, false if the material has already been registered
     */
    virtual bool registerMaterial(Material* material) = 0;

    /**
     * Draw a frame
     * @param deltaTime The length of the last frame in seconds
     */
    virtual void drawFrame(double deltaTime, double gameTime, const Scene& scene) = 0;

    /**
     * Cleanup the renderer so it's ready for destruction
     */
    virtual void cleanup();

    /**
     * Setup callbacks for GLFW Events
     */
     virtual void setupGlfwCallbacks();

    /**
     * Get if the windows wants to close
     */
    bool wantsToClose();

    GLFWwindow* getWindow();
};
