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
    VkInstance vInstance;                       // Vulkan instance handle
    VkDebugUtilsMessengerEXT debugMessenger;    // Debug output handle
    VkPhysicalDevice gpu;                       // Physical GPU Device Handle
    VkDevice device;                            // Device Handle for commands
    VkSurfaceKHR surface;                       // Window surface handle

    // Queues
    VkQueue graphicsQueue;              // The queue used for graphics commands

    VkCommandPool graphicsCommandPool;  // The command pool for graphics commands
    VkCommandBuffer commandBuffer;      // The command buffer

    // Swapchain
    VkSwapchainKHR swapchain;                       // Swap Chain handle
    VkFormat swapchainImageFormat;                  // The format of the swapchain
    std::vector<VkImage> swapchainImages;           // The images in the swapchain
    std::vector<VkImageView> swapchainImageViews;   // The image view wrappers

    // Renderpass
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> framebuffers;

    // Sync objects
    VkSemaphore presentSemaphore, renderSemaphore;
    VkFence renderFence;

    /**
     * Sets up the Vulkan instance, the device, and the queues
     * Populates vInstance, DebugMessenger, gpu, device, surface, graphicsQueue, graphicsCommandPool, and commandBuffer
     *
     * This function is monolithic in order to contain all of the vk_bootstrap content together without creating unnecessary class members
     * @param settings the engine settings
     * @return True if successful
     */
    bool initVulkan(EngineSettings& settings);

    /**
     * Sets up the swapchain
     * Populates swapchain, swapchainImageFormat, swapchainImages, and swapchainImageViews
     * @param settings the engine settings
     * @return True if successful
     */
    bool initSwapchain(EngineSettings& settings);

    /**
     * Setup the render pass
     * Populates renderpass
     * @param settings the engine settings
     * @return
     */
    bool initRenderpass(EngineSettings& settings);

    /**
     * Setup the framebuffers used for rendering to
     * Populates framebuffers
     * @param settings the engine settings
     * @return True if successful
     */
    bool initFramebuffers(EngineSettings& settings);

    /**
     * Initialise the semaphores and fences used to sync the rendering operations
     * Populates presentSemaphore, renderSemaphore, and renderFence
     * @param settings the engine settings
     * @return True if successful
     */
    bool initSyncObjects(EngineSettings& settings);

    /**
     * Initialise the shader pipeline
     * @param settings the engine settings
     * @return True if successful
     */
    bool initPipelines(EngineSettings& settings);

    /**
     * Load the shader at the given path
     * @param path The path to the shader on the disk
     * @param outShaderModule A pointer to store the created shader module in
     * @return True if successful
     */
    bool loadShader(const std::string& path, VkShaderModule* outShaderModule);

    /**
     * Cleanup the swapchain
     * This is separated out so the swapchain can be recreated without destroying the whole vulkan context
     */
    void cleanupSwapchain();
protected:
    /**
     * Required validation layers when running in debug mode
     */
    std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation",  // General vulkan debug
            "VK_LAYER_LUNARG_monitor"       // Adds an FPS counter to the title bar
    };

    /**
     * Required device extensions
     */
    std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME // The extension that enables the use of a swap chain for presenting
    };

    void setupGLFWHints() override;
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() override;

    // Overrides
    bool initialise(EngineSettings& settings) override;
    void drawFrame(double deltaTime, double gameTime) override;
    void cleanup() override;
};

