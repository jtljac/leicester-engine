/**
 * This code (and it's source) was created by following these guides:
 * <br>
 * https://vkguide.dev/             MIT Licence: https://github.com/vblanco20-1/vulkan-guide/blob/master/LICENSE.txt
 * https://vulkan-tutorial.com/     CCO 1.0 Universal Licence: https://github.com/Overv/VulkanTutorial/blob/master/README.md#license
 *
 * The guides have not been followed exactly, and modifications have been made to suit the project
 */

#pragma once

#define GLFW_INCLUDE_VULKAN

#include <vector>
#include <string>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <Utils/IdTrackedResource.h>
#include <Rendering/GPUStructures/GpuStructs.h>
#include <Scene/Scene.h>
#include <Mesh/Mesh.h>
#include <Mesh/StaticMesh.h>
#include <Texture/Texture.h>

#include "../Renderer.h"
#include "DeletionQueue.h"
#include "AllocationStructures.h"
#include "VMaterial.h"
#include "FrameData.h"
#include "VTexture.h"

struct TransferContext {
    VkFence transferFence = VK_NULL_HANDLE;
    VkSemaphore transferSemaphore = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer graphicsCommandBuffer = VK_NULL_HANDLE;
};

class VulkanRenderer : public Renderer {
    // Vulkan Handles
    VkInstance vInstance = VK_NULL_HANDLE;                       // Vulkan instance handle
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;    // Debug output handle
    VkPhysicalDevice gpu = VK_NULL_HANDLE;                       // Physical GPU Device Handle
    VkDevice device = VK_NULL_HANDLE;                            // Device Handle for commands
    VkSurfaceKHR surface = VK_NULL_HANDLE;                       // Window surface handle

    // GPU info
    VkPhysicalDeviceProperties gpuProperties;   // Info about the current GPU Device

    // Memory Management
    VmaAllocator allocator = VK_NULL_HANDLE;    // The GPU memory allocator

    // Queues
    uint32_t graphicsQueueIndex;                // The queue index used for the graphics queue
    VkQueue graphicsQueue = VK_NULL_HANDLE;     // The queue used for graphics commands

    uint32_t transferQueueIndex;                // The queue index used for the transfer queue
    VkQueue transferQueue = VK_NULL_HANDLE;     // The queue used for transfer commands

    // Swapchain
    VkSwapchainKHR swapchainHandle = VK_NULL_HANDLE;    // Swap Chain handle
    VkFormat swapchainImageFormat;                      // The format of the swapchainHandle

    // Depth Buffer
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;        // The format of the depth buffer

    // Renderpass
    VkRenderPass renderPass = VK_NULL_HANDLE;           // The renderpass handle

    // Descriptor Set
    VkDescriptorSetLayout globalDescriptorSetLayout = VK_NULL_HANDLE;   // Descriptor set layout for global data
    VkDescriptorSetLayout passDescriptorSetLayout = VK_NULL_HANDLE;     // Descriptor set layout for pass data
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;                   // Pool of which descriptor sets are allocated to

    // Scene data
    GPUSceneData sceneParams{};
    AllocatedBuffer sceneParamsBuffer{};

    // Frame Data
    unsigned int currentFrame = 0;              // The current frame being rendered
    std::vector<SwapchainData> swapchainData;   // An array containing the data of each frame of the swapchainHandle
    std::vector<FrameData> frameData;           // An array containing the data of each frame

    DeletionQueue deletionQueue;    // A queue storing deletion functions

    // GPU Memory Trackers
    IDTrackedResource<uint64_t, AllocatedBuffer> bufferList;    // Stores Allocated Buffers against an ID
    IDTrackedResource<uint64_t, VTexture> imageList;      // Stores Allocated Images against an ID
    IDTrackedResource<uint64_t, VMaterial> materialList;        // Stores Materials against an ID
    Material collisionMat = Material("/Colliders/Collider.vert.spv", "/Colliders/Collider.frag.spv", ShaderType::WIREFRAME);

    // Transfer
    TransferContext transferContext{};    // The object containing transfer structures

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

    /**
     * Pad the given size so it's aligned to the GPU's alignment boundaries
     * <br>
     * Taken from https://github.com/SaschaWillems/Vulkan/tree/master/examples/dynamicuniformbuffer    MIT License: https://github.com/SaschaWillems/Vulkan/blob/master/LICENSE.md
     * @param originalSize The original size of the data being padded
     * @return a new size, aligned to the GPU's alignment boundaries
     */
    size_t padUniformBufferSize(size_t originalSize);

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
     * Setup the descriptors used by the pipelines
     * @param settings the engine settings
     */
    bool initDescriptors(EngineSettings& settings);

    /**
     * Initialises the frameData objects
     * @param settings the engine settings
     * @param graphicsQueueIndex the index of the graphics queue used for the command pools
     * @return True if successful
     */
    bool initFrameData(EngineSettings& settings);

    /**
     * Initialises the graphics pools in the provided FrameData
     * Populates commandPool and commandBuffer
     * @param settings the engine settings
     * @param frameData the FrameData object being populated
     * @param graphicsQueueIndex the index of the graphics queue used for the command pools
     * @return True if successful
     */
    bool initFrameDataGraphicsPools(EngineSettings& settings, FrameData& frameData);

    /**
     * Initialise the semaphores and fences of the given FrameData
     * Populates presentSemaphore, renderSemaphore, and renderFence
     * @param settings the engine settings
     * @param frameData the FrameData object being populated
     * @return True if successful
     */
    bool initFrameDataSyncObjects(EngineSettings& settings, FrameData& frameData);

    /**
     * Initialises the description sets of the given FrameData
     * Populates cameraBuffer
     * @param settings the engine settings
     * @param frameData the FrameData object being populated
     * @return True if successful
     */
    void initFrameDataDescriptorSets(EngineSettings& settings, FrameData& frameData);

    /**
     * Initialises the structures used for transfer operations between the CPU and GPU
     * Populates transferContext
     * @param settings the engine settings
     * @return True if successful
     */
    bool initTransferContext(EngineSettings& settings);

    /**
     * Sets up the swapchainHandle
     * Populates swapchainHandle, swapchainImageFormat, swapchainImages, and swapchainImageViews
     * @param settings the engine settings
     * @return True if successful
     */
    bool initSwapchain(EngineSettings& settings);

    /**
     * Sets up the depth buffers for the given SwapchainData
     * Populates depthImage & depthImageView
     * @param settings the engine settings
     * @param swapchain the SwapchainData being populated
     * @return True if successful
     */
    bool initSwapchainDepthBuffer(EngineSettings& settings, SwapchainData& swapchain);

    /**
     * Setup the framebuffers used for rendering to for the given SwapchainData
     * Populates framebuffer
     * @param settings the engine settings
     * @param swapchain the SwapchainData being populated
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

    // Resource Handling

    /**
     * Immediately submit commands on the transfer queue
     * @param function A function that executes commands on the commandBuffer
     */
    [[maybe_unused]] VkResult executeTransfer(std::function<VkResult(VkCommandBuffer commandBuffer)>&& function);

    /**
     * Immediately submit commands on the transfer and graphics queue, syncronised to ensure the transfer queue executes
     * first
     * <br>
     * Note this does not begin or end either buffer
     * @param function A function that executes commands on the command buffers
     * @return
     */
    [[maybe_unused]] VkResult executeTransfer(std::function<VkResult(VkCommandBuffer transferBuffer, VkCommandBuffer graphicsBuffer)>&& function);

    /**
     * Load the shader at the given path
     * @param path The path to the shader on the disk
     * @param outShaderModule A pointer to store the created shader module in
     * @return True if successful
     */
    bool loadShader(const std::string& path, VkShaderModule* outShaderModule);

    /**
     * Allocate GPU memory
     * @param allocSize The amount of memory to allocated
     * @param usage The buffer usage flags
     * @param flags The allocation flags
     * @param memoryUsage The vma Memory usage flags
     * @return The allocated buffer representing the allocated gpu memory
     */
    AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);

    /**
     * Allocate an image on the GPU
     * @param imageCreateInfo The image info
     * @param flags The allocation flags
     * @param memoryUsage The VMA Memory usage flags
     * @return The allocated image representing the allocated gpu image
     */
    AllocatedImage createImage(VkImageCreateInfo& imageCreateInfo, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);

    /**
     * Upload a mesh to the GPU
     * Sets the vertices and indices ids of the mesh
     * @param mesh The mesh to upload
     */
    void uploadMesh(Mesh& mesh);

    /**
     * Upload a transferCommandBuffer to the GPU
     * Sets the textureId of the transferCommandBuffer
     * @param transferCommandBuffer The transferCommandBuffer to upload
     */
    void uploadTexture(Texture& transferCommandBuffer);

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
     * Cleanup the swapchainHandle
     * This is separated out so the swapchainHandle can be recreated without destroying the whole vulkan context
     */
    void cleanupSwapchain();

protected:
    void setupGLFWHints() override;
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() override = default;

    // Overrides
    bool initialise(EngineSettings& settings) override;
    void drawFrame(double deltaTime, double gameTime, const Scene& scene) override;
    void cleanup() override;

    // Resource Management
    void setupScene(Scene& scene) override;
    bool registerMesh(Mesh* mesh) override;
    bool registerTexture(Texture* texture) override;
    bool registerMaterial(Material* material) override;
};

