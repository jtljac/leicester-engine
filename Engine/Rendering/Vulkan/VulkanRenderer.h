//
// Created by jacob on 21/10/22.
//

#pragma once

#include <vector>
#include <string>

#include "../Renderer.h"

#include <vulkan/vulkan.h>

class VulkanRenderer : public Renderer {
    // Vulkan Handles
    VkInstance vInstance; // Vulkan instance handle
    VkDebugUtilsMessengerEXT debugMessenger; // Debug output handle
    VkPhysicalDevice gpu; // Physical GPU Device Handle
    VkDevice device; // Device Handle for commands
    VkSurfaceKHR surface; // Window surface handle

    /**
     * Sets up the Vulkan instance and the device
     * Populates vInstance, DebugMessenger, physical device, and surface
     * @return True if successful
     */
    bool initVulkan(EngineSettings& settings);
protected:
    // Required validation layers when running in debug mode
    std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation",  // General vulkan debug
            "VK_LAYER_LUNARG_monitor"       // Adds an FPS counter to the titlebar
    };

    std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME // The extension that enables the use of a swap chain for presenting
    };

    void setupGLFWHints() override;
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() override;
    bool initialise(EngineSettings& settings) override;
    void drawFrame(double deltaTime) override;
};

