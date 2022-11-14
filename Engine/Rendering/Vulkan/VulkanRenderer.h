/**
 * This code (and it's source) was created by following these guides:
 * https://vkguide.dev/             MIT Licence: https://github.com/vblanco20-1/vulkan-guide/blob/master/LICENSE.txt
 * https://vulkan-tutorial.com/     CCO 1.0 Universal Licence: https://github.com/Overv/VulkanTutorial/blob/master/README.md#license
 *
 * The guides have not been followed exactly, and modifications have been made to suit the project
 */

#pragma once

#include <vector>
#include <string>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "../Renderer.h"
#include <Mesh/Mesh.h>
#include "DeletionQueue.h"
#include "AllocationStructures.h"
#include "Utils/IdTrackedResource.h"
#include "VMaterial.h"
#include "Mesh/StaticMesh.h"
#include "FrameData.h"

class VulkanRenderer : public Renderer {
    // Vulkan Handles
    VkInstance vInstance;                       // Vulkan instance handle
    VkDebugUtilsMessengerEXT debugMessenger;    // Debug output handle
    VkPhysicalDevice gpu;                       // Physical GPU Device Handle
    VkDevice device;                            // Device Handle for commands
    VkSurfaceKHR surface;                       // Window surface handle

    // Memory Management
    VmaAllocator allocator;                     // The GPU memory allocator

    // Queues
    VkQueue graphicsQueue;              // The queue used for graphics commands

    // Swapchain
    VkSwapchainKHR swapchain;                       // Swap Chain handle
    VkFormat swapchainImageFormat;                  // The format of the swapchain

    // Depth Buffer
    VkFormat depthFormat;

    // Renderpass
    VkRenderPass renderPass;                    // The renderpass handle

    // Frame Data
    unsigned int currentFrame = 0;
    std::vector<SwapchainData> swapchainData;
    std::vector<FrameData> frameData;


    DeletionQueue deletionQueue;        // A queue storing deletion functions

    // GPU Memory Trackers
    IDTrackedResource<uint64_t, AllocatedBuffer> bufferList;    // Stores Allocated Buffers against an ID
    IDTrackedResource<uint64_t, VMaterial> materialList;        // Stores Materials against an ID

    std::vector<StaticMesh> renderables;

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

private:

    /**
     * Sets up the Vulkan instance, the device, and the queues
     * Populates vInstance, DebugMessenger, gpu, device, surface, graphicsQueue, and frameData
     *
     * This function is monolithic in order to contain all of the vk_bootstrap content together without creating unnecessary class members
     * @param settings the engine settings
     * @return True if successful
     */
    bool initVulkan(EngineSettings& settings);

    /**
     * Initialises the frameData objects
     * @param settings the engine settings
     * @param graphicsQueueIndex the index of the graphics queue used for the command pools
     * @return True if successful
     */
    bool initFrameData(EngineSettings& settings, unsigned int graphicsQueueIndex);

    /**
     * Initialises the graphics pools in the provided FrameData
     * Populates commandPool and commandBuffer
     * @param settings the engine settings
     * @param frameData the FrameData object being populated
     * @param graphicsQueueIndex the index of the graphics queue used for the command pools
     * @return True if successful
     */
    bool initFrameDataGraphicsPools(EngineSettings& settings, FrameData& frameData, unsigned int graphicsQueueIndex);

    /**
     * Initialise the semaphores and fences of the given frameData
     * Populates presentSemaphore, renderSemaphore, and renderFence
     * @param settings the engine settings
     * @param frameData the FrameData object being populated
     * @return True if successful
     */
    bool initFrameDataSyncObjects(EngineSettings& settings, FrameData& frameData);

    /**
     * Sets up the swapchain
     * Populates swapchain, swapchainImageFormat, swapchainImages, and swapchainImageViews
     * @param settings the engine settings
     * @return True if successful
     */
    bool initSwapchain(EngineSettings& settings);

    /**
     * Setup the framebuffers used for rendering to
     * Populates framebuffers
     * @param settings the engine settings
     * @return True if successful
     */
    bool initFramebuffers(EngineSettings& settings);

    /**
     * Setup the render pass
     * Populates renderpass
     * @param settings the engine settings
     * @return
     */
    bool initRenderpass(EngineSettings& settings);

    /**
     * Initialise the shader pipeline
     * @param settings the engine settings
     * @return True if successful
     */
    bool initRender(EngineSettings& settings);

    /**
     * Load the shader at the given path
     * @param path The path to the shader on the disk
     * @param outShaderModule A pointer to store the created shader module in
     * @return True if successful
     */
    bool loadShader(const std::string& path, VkShaderModule* outShaderModule);

    /**
     * Upload a mesh to the GPU
     * Sets the vertices and indices ids of the mesh
     * @param mesh The mesh to upload
     * @return True if successful
     */
    bool uploadMesh(Mesh& mesh);

    /**
     * Uploads a material to the GPU
     * Sets the materialId of the material
     * @param material the material to upload
     * @return True if successful
     */
    bool createMaterial(Material& material);

    /**
     * Gets the frameData of the current frame
     * @return a reference to the frameData of the current frame
     */
    FrameData& getCurrentFrame();

    /**
     * Cleanup the swapchain
     * This is separated out so the swapchain can be recreated without destroying the whole vulkan context
     */
    void cleanupSwapchain();
protected:
    void setupGLFWHints() override;
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() override;

    // Overrides
    bool initialise(EngineSettings& settings) override;
    void drawFrame(double deltaTime, double gameTime) override;
    void cleanup() override;
};

