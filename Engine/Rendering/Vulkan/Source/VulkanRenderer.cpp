#include <cmath>
#include <fstream>

#include "../VulkanRenderer.h"
#include "../PipelineBuilder.h"
#include "../MeshPushConstants.h"
#include <Utils/Logger.h>
#include <Utils/FileUtils.h>

#include <VkBootstrap.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "../VkShortcuts.h"

#include <Rendering/Vulkan/VTexture.h>

/* ========================================= */
/* Setup                                     */
/* ========================================= */

void VulkanRenderer::setupGLFWHints() {
    Renderer::setupGLFWHints();
    // Since we're using vulkan, tell it not to create an opengl context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Prevent resizing the window
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

/* ========================================= */
/* Initialisation                            */
/* ========================================= */

bool VulkanRenderer::initialise(EngineSettings& settings) {
    Renderer::initialise(settings);

    if (!this->initVulkan(settings)) return false;
    if (!this->initSwapchain(settings)) return false;

    if (!this->initGBuffers(settings)) return false;

    if (!this->initSamplers(settings)) return false;

    if (!this->initDescriptors(settings)) return false;
    if (!this->initFrameData(settings)) return false;
    if (!this->deferredSyncObjects(settings)) return false;

    if (!this->initTransferContext(settings)) return false;

    if (!this->initDeferredRenderpass(settings)) return false;
    if (!this->initDeferredFramebuffers(settings)) return false;

    if (!this->initCombinationRenderpass(settings)) return false;
    if (!this->initCombinationFramebuffers(settings)) return false;

    // Initialise collision visualisation
    createMaterial(this->collisionMat);
    createMaterial(this->combinationMat);

    return true;
}

bool VulkanRenderer::initVulkan(EngineSettings& settings) {
    vkb::Instance vkbInstance;
    // Setup Vulkan Instance
    {
        vkb::InstanceBuilder instanceBuilder;
        instanceBuilder
                .set_engine_name("leicester-engine")
                .set_app_name(settings.windowTitle.c_str())
                .request_validation_layers(true)
                .require_api_version(1, 1, 0)
                .use_default_debug_messenger();

        auto systemInfo = vkb::SystemInfo::get_system_info();
        if (!systemInfo.has_value()) {
            Logger::error(systemInfo.error().message());
            return false;
        }
        for (const auto& layer: this->validationLayers) {
            if (systemInfo->is_layer_available(layer)) instanceBuilder.enable_layer(layer);
            else {
                // Need to construct the message in a silly way so we can concat the layer on the end
                std::string message = "Failed to find validation layer: ";
                message += layer;
                Logger::warn(message);
            }
        }

        uint32_t extensionCount;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        for (int i = 0; i < extensionCount; ++i) {
            if (systemInfo->is_extension_available(glfwExtensions[i])) instanceBuilder.enable_extension(glfwExtensions[i]);
            else {
                // Need to construct the message in a silly way so we can concat the layer on the end
                std::string message = "Failed to find validation layer: ";
                message += glfwExtensions[i];
                Logger::warn(message);
            }
        }

        auto vkbInst = instanceBuilder
                .build();

        if (!vkbInst.has_value()) {
            Logger::error("Failed to create instance");
            return false;
        }

        vkbInstance = vkbInst.value();

        this->vInstance = vkbInstance.instance;
        this->debugMessenger = vkbInstance.debug_messenger;
    }

    // Surface
    {
        VkResult result = glfwCreateWindowSurface(this->vInstance, this->window, nullptr, &surface);

        if (result != VK_SUCCESS) {
            Logger::error("Failed to create Surface");
            return false;
        }
    }

    // Setup Device
    vkb::Device vkbDevice;
    {
        VkPhysicalDeviceFeatures features{};
        features.fillModeNonSolid = VK_TRUE;
        auto deviceSelector = vkb::PhysicalDeviceSelector(vkbInstance)
                .set_minimum_version(1, 1)
                .set_surface(surface)
                .set_required_features(features)
                .add_required_extensions(this->deviceExtensions)
                .select();

        if (!deviceSelector.has_value()) {
            Logger::error("Failed to select device");
            return false;
        }

        vkb::PhysicalDevice vkbPDevice = deviceSelector.value();

        VkPhysicalDeviceShaderDrawParametersFeatures drawParametersFeatures = {};
        drawParametersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
        drawParametersFeatures.pNext = nullptr;
        drawParametersFeatures.shaderDrawParameters = VK_TRUE;

        auto vkbDev = vkb::DeviceBuilder(vkbPDevice)
                .add_pNext(&drawParametersFeatures)
                .build();

        if (!deviceSelector.has_value()) {
            Logger::error("Failed to create logical device");
            return false;
        }

        vkbDevice = vkbDev.value();

        this->device = vkbDevice.device;
        this->gpu = vkbDevice.physical_device;
        this->gpuProperties = vkbDevice.physical_device.properties;

        this->graphicsQueueIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
        this->graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();

        auto vkbTransferQueueIndex = vkbDevice.get_queue_index(vkb::QueueType::transfer);
        if (vkbTransferQueueIndex.has_value()) {
            this->transferQueueIndex = vkbTransferQueueIndex.value();
            this->transferQueue = vkbDevice.get_queue(vkb::QueueType::transfer).value();
        } else {
            Logger::warn("This device does not have a separate transfer queue, defaulting to the graphics queue");
            this->transferQueueIndex = this->graphicsQueueIndex;
            this->transferQueue = this->graphicsQueue;
        }
    }

    // VMA Allocator
    {
        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.physicalDevice = this->gpu;
        allocatorCreateInfo.device = this->device;
        allocatorCreateInfo.instance = this->vInstance;

        if (vmaCreateAllocator(&allocatorCreateInfo, &this->allocator) != VK_SUCCESS){
            Logger::error("Failed to create memory allocator");
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::initSwapchain(EngineSettings& settings) {
    vkb::SwapchainBuilder swapchainBuilder(this->gpu, this->device, this->surface);

    auto vkbSc = swapchainBuilder
            .use_default_format_selection()
            .set_desired_extent(settings.windowWidth, settings.windowHeight)
            .set_desired_min_image_count(settings.bufferCount)
            .set_desired_present_mode(VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
            .add_fallback_present_mode(VkPresentModeKHR::VK_PRESENT_MODE_IMMEDIATE_KHR)
            .build();

    if (!vkbSc.has_value()) {
        Logger::error("Failed to create swap chain");
        Logger::error(vkbSc.error().message());
        return false;
    }

    vkb::Swapchain vkbSwapchain = vkbSc.value();

    this->swapchainHandle = vkbSwapchain.swapchain;
    this->swapchainImageFormat = vkbSwapchain.image_format;

    auto swapchainImages = vkbSwapchain.get_images().value();
    auto swapchainImageViews = vkbSwapchain.get_image_views().value();

    this->swapchainData.resize(swapchainImages.size());
    for (int i = 0; i < swapchainImages.size(); ++i) {
        this->swapchainData[i].swapchainImage = swapchainImages[i];
        this->swapchainData[i].swapchainImageView = swapchainImageViews[i];
    }
    return true;
}

bool VulkanRenderer::initGBuffers(EngineSettings& settings) {
    VkExtent3D gBufferExtent{settings.windowWidth, settings.windowHeight, 1};

    GBufferData* iter[] = {
        &this->deferredFrameData.position,
        &this->deferredFrameData.albedo,
        &this->deferredFrameData.metallicRoughnessAO,
        &this->deferredFrameData.normal
    };

    // Position, albedo, normal
    for (GBufferData* gBufferData: iter) {
        gBufferData->format = VK_FORMAT_R8G8B8A8_SRGB;
        VKShortcuts::createAllocatedImage(this->allocator,
                                          gBufferData->format,
                                          gBufferExtent,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                          VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                                          gBufferData->image);
        
        VKShortcuts::createImageView(this->device,
                                     gBufferData->format,
                                     gBufferData->image.image,
                                     VK_IMAGE_ASPECT_COLOR_BIT,
                                     gBufferData->imageView);
    }

    // depth
    {
        GBufferData* gBufferData = &this->depthBuffer;
        gBufferData->format = VK_FORMAT_D32_SFLOAT;
        VKShortcuts::createAllocatedImage(this->allocator,
                                          gBufferData->format,
                                          gBufferExtent,
                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                          VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                                          gBufferData->image);

        VKShortcuts::createImageView(this->device,
                                     gBufferData->format,
                                     gBufferData->image.image,
                                     VK_IMAGE_ASPECT_DEPTH_BIT,
                                     gBufferData->imageView);
    }

    return true;
}

bool VulkanRenderer::initSamplers(EngineSettings& settings) {
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext = nullptr;

    samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    return vkCreateSampler(device, &samplerCreateInfo, nullptr, &this->colourSampler) == VK_SUCCESS;
}

bool VulkanRenderer::initDescriptors(EngineSettings& settings) {
    // Descriptor Pool
    std::vector<VkDescriptorPoolSize> sizes = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.pNext = nullptr;

    descriptorPoolCreateInfo.flags = 0;
    descriptorPoolCreateInfo.maxSets = 10;
    descriptorPoolCreateInfo.poolSizeCount = (uint32_t) sizes.size();
    descriptorPoolCreateInfo.pPoolSizes = sizes.data();

    vkCreateDescriptorPool(this->device, &descriptorPoolCreateInfo, nullptr, &this->descriptorPool);

    // Global Descriptor
    {
        const size_t sceneParamBufferSize = settings.bufferCount * padUniformBufferSize(sizeof(GPUSceneData));
        sceneParamsBuffer = createBuffer(sceneParamBufferSize,
                                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                         VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        std::array<VkDescriptorSetLayoutBinding, 3> bindings{{
            VKShortcuts::createDescriptorSetLayoutBinding(0,
                                                          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                          VK_SHADER_STAGE_VERTEX_BIT),

            VKShortcuts::createDescriptorSetLayoutBinding(1,
                                                          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                          VK_SHADER_STAGE_VERTEX_BIT |
                                                          VK_SHADER_STAGE_FRAGMENT_BIT),
            VKShortcuts::createDescriptorSetLayoutBinding(2,
                                                          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                                          VK_SHADER_STAGE_VERTEX_BIT |
                                                          VK_SHADER_STAGE_FRAGMENT_BIT)
        }};

        VkDescriptorSetLayoutCreateInfo globalDescriptorSetLayoutCreateInfo = {};
        globalDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        globalDescriptorSetLayoutCreateInfo.pNext = nullptr;
        globalDescriptorSetLayoutCreateInfo.flags = 0;

        globalDescriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        globalDescriptorSetLayoutCreateInfo.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(this->device, &globalDescriptorSetLayoutCreateInfo, nullptr,
                                    &this->globalDescriptorSetLayout) != VK_SUCCESS) {
            Logger::error("Failed to create globalDescriptorSetLayout");
            return false;
        }
    }

    // Deferred Pass Descriptor
    {
        std::array<VkDescriptorSetLayoutBinding, 1> bindings{{
            VKShortcuts::createDescriptorSetLayoutBinding(0,
                                                          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                          VK_SHADER_STAGE_VERTEX_BIT)
        }};

        VkDescriptorSetLayoutCreateInfo passDescriptorSetLayoutCreateInfo = {};
        passDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        passDescriptorSetLayoutCreateInfo.pNext = nullptr;
        passDescriptorSetLayoutCreateInfo.flags = 0;

        passDescriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        passDescriptorSetLayoutCreateInfo.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(this->device, &passDescriptorSetLayoutCreateInfo, nullptr,
                                    &this->deferredPassDescriptorSetLayout) != VK_SUCCESS){
            Logger::error("Failed to create deferred pass DescriptorSetLayout");
            return false;
        }
    }

    // Combination Pass Descriptor
    {
        std::array<VkDescriptorSetLayoutBinding, 5> bindings{{
                VKShortcuts::createDescriptorSetLayoutBinding(0,
                                                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                            VK_SHADER_STAGE_FRAGMENT_BIT),
                VKShortcuts::createDescriptorSetLayoutBinding(1,
                                                              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                              VK_SHADER_STAGE_FRAGMENT_BIT),
                VKShortcuts::createDescriptorSetLayoutBinding(2,
                                                              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                              VK_SHADER_STAGE_FRAGMENT_BIT),
                VKShortcuts::createDescriptorSetLayoutBinding(3,
                                                              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                              VK_SHADER_STAGE_FRAGMENT_BIT),
                VKShortcuts::createDescriptorSetLayoutBinding(4,
                                                              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                              VK_SHADER_STAGE_FRAGMENT_BIT),
        }};

        VkDescriptorSetLayoutCreateInfo passDescriptorSetLayoutCreateInfo = {};
        passDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        passDescriptorSetLayoutCreateInfo.pNext = nullptr;
        passDescriptorSetLayoutCreateInfo.flags = 0;

        passDescriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        passDescriptorSetLayoutCreateInfo.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(this->device, &passDescriptorSetLayoutCreateInfo, nullptr,
                                        &this->combinationPassDescriptorSetLayout) != VK_SUCCESS){
            Logger::error("Failed to create deferred pass DescriptorSetLayout");
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::initFrameData(EngineSettings& settings) {
    frameData.resize(settings.bufferCount);

    for (int i = 0; i < frameData.size(); ++i) {
        FrameData& data = this->frameData[i];
        if (!initFrameDataGraphicsPools(settings, data)) return false;

        if (!initFrameDataSyncObjects(settings, data)) return false;

        initFrameDataDescriptorSets(settings, data);
    }
    return true;
}

bool VulkanRenderer::initFrameDataGraphicsPools(EngineSettings& settings, FrameData& frameData) {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;

    commandPoolCreateInfo.queueFamilyIndex = this->graphicsQueueIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(this->device, &commandPoolCreateInfo, nullptr, &frameData.commandPool) != VK_SUCCESS) {
        Logger::error("Failed to create frameData command pool");
        return false;
    }


    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;

    commandBufferAllocateInfo.commandPool = frameData.commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 2;

    VkCommandBuffer buffers[2];

    if (vkAllocateCommandBuffers(this->device, &commandBufferAllocateInfo, buffers) != VK_SUCCESS) {
        Logger::error("Failed to create frameData command buffer");
        return false;
    }

    frameData.deferredCommandBuffer = buffers[0];
    frameData.combinationCommandBuffer = buffers[1];

    return true;
}

bool VulkanRenderer::initFrameDataSyncObjects(EngineSettings& settings, FrameData& frameData) {
    // Fence
    {
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(this->device, &fenceCreateInfo, nullptr, &frameData.renderFence) != VK_SUCCESS) {
            Logger::error("Failed to create frameData fence");
            return false;
        }
    }
    // Semaphores
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;
        semaphoreCreateInfo.flags = 0;

        if (vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr, &frameData.renderSemaphore) != VK_SUCCESS
            || vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr, &frameData.presentSemaphore) != VK_SUCCESS) {
            Logger::error("Failed to create frameData semaphore");
            return false;
        }
    }
    return true;
}

void
VulkanRenderer::initFrameDataDescriptorSets(EngineSettings& settings, FrameData& frameData) {
    constexpr int maxObjectCount = 10000;

    frameData.cameraBuffer = createBuffer(sizeof(GpuCameraData),
                                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    frameData.objectBuffer = createBuffer(sizeof(GpuObjectData) * maxObjectCount,
                                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    // Global Descriptor
    {
        VkDescriptorSetAllocateInfo globalDescriptorSetAllocateInfo = {};
        globalDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        globalDescriptorSetAllocateInfo.pNext = nullptr;

        globalDescriptorSetAllocateInfo.descriptorPool = this->descriptorPool;
        globalDescriptorSetAllocateInfo.descriptorSetCount = 1;
        globalDescriptorSetAllocateInfo.pSetLayouts = &this->globalDescriptorSetLayout;

        vkAllocateDescriptorSets(this->device, &globalDescriptorSetAllocateInfo, &frameData.globalDescriptor);
    }

    // Deferred Pass Descriptor
    {
        VkDescriptorSetAllocateInfo passDescriptorSetAllocateInfo = {};
        passDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        passDescriptorSetAllocateInfo.pNext = nullptr;

        passDescriptorSetAllocateInfo.descriptorPool = this->descriptorPool;
        passDescriptorSetAllocateInfo.descriptorSetCount = 1;
        passDescriptorSetAllocateInfo.pSetLayouts = &this->deferredPassDescriptorSetLayout;

        vkAllocateDescriptorSets(this->device, &passDescriptorSetAllocateInfo, &frameData.deferredPassDescriptor);
    }

    VkDescriptorBufferInfo cameraMatInfo;
    cameraMatInfo.buffer = frameData.cameraBuffer.buffer;
    cameraMatInfo.offset = 0;
    cameraMatInfo.range = sizeof(GpuCameraMat);

    VkDescriptorBufferInfo cameraMetaInfo;
    cameraMetaInfo.buffer = frameData.cameraBuffer.buffer;
    cameraMetaInfo.offset = sizeof(GpuCameraMat);
    cameraMetaInfo.range = sizeof(GpuCameraMeta);

    VkDescriptorBufferInfo sceneInfo;
    sceneInfo.buffer = sceneParamsBuffer.buffer;
    sceneInfo.offset = 0;
    sceneInfo.range = sizeof(GPUSceneData);

    VkDescriptorBufferInfo objectInfo;
    objectInfo.buffer = frameData.objectBuffer.buffer;
    objectInfo.offset = 0;
    objectInfo.range = sizeof(GpuObjectData) * maxObjectCount;

    std::array<VkWriteDescriptorSet, 4> setWrites {{
            VKShortcuts::createWriteDescriptorSet(0, frameData.globalDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &cameraMatInfo),
            VKShortcuts::createWriteDescriptorSet(1, frameData.globalDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &cameraMetaInfo),
            VKShortcuts::createWriteDescriptorSet(2, frameData.globalDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, &sceneInfo),
            VKShortcuts::createWriteDescriptorSet(0, frameData.deferredPassDescriptor, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &objectInfo)
    }};

    vkUpdateDescriptorSets(this->device, setWrites.size(), setWrites.data(), 0, nullptr);

    // Combination Pass Descriptor
    {
        VkDescriptorSetAllocateInfo passDescriptorSetAllocateInfo = {};
        passDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        passDescriptorSetAllocateInfo.pNext = nullptr;

        passDescriptorSetAllocateInfo.descriptorPool = this->descriptorPool;
        passDescriptorSetAllocateInfo.descriptorSetCount = 1;
        passDescriptorSetAllocateInfo.pSetLayouts = &this->combinationPassDescriptorSetLayout;

        vkAllocateDescriptorSets(this->device, &passDescriptorSetAllocateInfo, &frameData.combinationPassDescriptor);

        VkDescriptorImageInfo position = {
                this->colourSampler,
                deferredFrameData.position.imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        VkDescriptorImageInfo albedo = {
                this->colourSampler,
                deferredFrameData.albedo.imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        VkDescriptorImageInfo metallicRoughnessAo = {
                this->colourSampler,
                deferredFrameData.metallicRoughnessAO.imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        VkDescriptorImageInfo normal = {
                this->colourSampler,
                deferredFrameData.normal.imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        VkDescriptorImageInfo depth = {
                this->colourSampler,
                depthBuffer.imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        std::array<VkWriteDescriptorSet, 5> setWrites {
            VKShortcuts::createWriteDescriptorSetImage(0, frameData.combinationPassDescriptor, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &position),
            VKShortcuts::createWriteDescriptorSetImage(1, frameData.combinationPassDescriptor, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &albedo),
            VKShortcuts::createWriteDescriptorSetImage(1, frameData.combinationPassDescriptor, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &metallicRoughnessAo),
            VKShortcuts::createWriteDescriptorSetImage(2, frameData.combinationPassDescriptor, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &normal),
            VKShortcuts::createWriteDescriptorSetImage(3, frameData.combinationPassDescriptor, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &depth)
        };

        vkUpdateDescriptorSets(this->device, setWrites.size(), setWrites.data(), 0, nullptr);
    }
}

bool VulkanRenderer::deferredSyncObjects(EngineSettings& settings) {
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;

    if (vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr, &deferredFrameData.deferredSemaphore) != VK_SUCCESS) {
        Logger::error("Failed to create deferred semaphore");
        return false;
    }

    return true;
}

bool VulkanRenderer::initTransferContext(EngineSettings& settings) {
    // Fence
    {
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;

        if (vkCreateFence(this->device, &fenceCreateInfo, nullptr, &transferContext.transferFence) != VK_SUCCESS) {
            Logger::error("Failed to create transferContext fence");
            return false;
        }
    }

    // Semaphore
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;

        if (vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr, &transferContext.transferSemaphore) != VK_SUCCESS) {
            Logger::error("Failed to create transferContext semaphore");
            return false;
        }
    }

    // Command Pools
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.pNext = nullptr;

        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        // Transfer Pool
        commandPoolCreateInfo.queueFamilyIndex = this->transferQueueIndex;
        if (vkCreateCommandPool(this->device, &commandPoolCreateInfo, nullptr, &transferContext.commandPool) != VK_SUCCESS) {
            Logger::error("Failed to create transferContext transfer command pool");
            return false;
        }

        // Graphics Pool
        commandPoolCreateInfo.queueFamilyIndex = this->graphicsQueueIndex;
        if (vkCreateCommandPool(this->device, &commandPoolCreateInfo, nullptr, &globalGraphicsContext.commandPool) != VK_SUCCESS) {
            Logger::error("Failed to create transferContext graphics command pool");
            return false;
        }

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.pNext = nullptr;

        commandBufferAllocateInfo.commandBufferCount = 1;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        // Transfer Buffer
        commandBufferAllocateInfo.commandPool = transferContext.commandPool;
        if (vkAllocateCommandBuffers(this->device, &commandBufferAllocateInfo, &transferContext.commandBuffer) != VK_SUCCESS) {
            Logger::error("Failed to create transferContext transfer command buffer");
            return false;
        }

        // Graphics Buffer
        commandBufferAllocateInfo.commandPool = globalGraphicsContext.commandPool;
        if (vkAllocateCommandBuffers(this->device, &commandBufferAllocateInfo, &globalGraphicsContext.commandBuffer) != VK_SUCCESS) {
            Logger::error("Failed to create transferContext graphics command buffer");
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::initDeferredRenderpass(EngineSettings& settings) {
    std::array<VkAttachmentDescription, 5> attachments{{
        {
                0,
                this->deferredFrameData.position.format,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
                0,
                this->deferredFrameData.albedo.format,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
                0,
                this->deferredFrameData.metallicRoughnessAO.format,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
                0,
                this->deferredFrameData.normal.format,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        {
                0,
                this->depthBuffer.format,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
        }
    }};

    std::array<VkAttachmentReference, 4> attachmentReferences = {{
            {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
            {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
            {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
            {3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    }};
    VkAttachmentReference depthAttachmentReference{4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    std::array<VkSubpassDependency, 4> subpassDependencies = {{

            {
                    VK_SUBPASS_EXTERNAL,
                    0,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    0,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_DEPENDENCY_BY_REGION_BIT
            },
            // Ensure depth doesn't overlap
            {
                    VK_SUBPASS_EXTERNAL,
                    0,
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    0,
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    VK_DEPENDENCY_BY_REGION_BIT
            },
            // Transition GBuffers to readable
            {
                    0,
                    VK_SUBPASS_EXTERNAL,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_ACCESS_MEMORY_READ_BIT,
                    VK_DEPENDENCY_BY_REGION_BIT
            },
            // Transition depth buffer to something readable in a shader
            {
                    0,
                    VK_SUBPASS_EXTERNAL,
                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    VK_ACCESS_SHADER_READ_BIT,
                    VK_DEPENDENCY_BY_REGION_BIT
            }
    }};

    VkSubpassDescription subpass = {};
    {
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = attachmentReferences.size();
        subpass.pColorAttachments = attachmentReferences.data();
        subpass.pDepthStencilAttachment = &depthAttachmentReference;
    }

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    {
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();

        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;

        renderPassCreateInfo.dependencyCount = subpassDependencies.size();
        renderPassCreateInfo.pDependencies = subpassDependencies.data();
    }

    if (vkCreateRenderPass(this->device, &renderPassCreateInfo, nullptr, &deferredRenderpass) != VK_SUCCESS) {
        Logger::error("Failed to create deferred renderpass");
        return false;
    }
    return true;
}

bool VulkanRenderer::initDeferredFramebuffers(EngineSettings& settings) {
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;

    framebufferCreateInfo.renderPass = this->deferredRenderpass;

    std::array<VkImageView, 5> attachments{{
        this->deferredFrameData.position.imageView,
        this->deferredFrameData.albedo.imageView,
        this->deferredFrameData.metallicRoughnessAO.imageView,
        this->deferredFrameData.normal.imageView,
        this->depthBuffer.imageView
    }};

    framebufferCreateInfo.attachmentCount = attachments.size();
    framebufferCreateInfo.pAttachments = attachments.data();

    framebufferCreateInfo.width = settings.windowWidth;
    framebufferCreateInfo.height = settings.windowHeight;
    framebufferCreateInfo.layers = 1;


    if (vkCreateFramebuffer(this->device, &framebufferCreateInfo, nullptr, &this->deferredFrameData.framebuffer) != VK_SUCCESS) {
        Logger::error("Failed to create deferred framebuffer");
        return false;
    }

    return true;
}

bool VulkanRenderer::initCombinationRenderpass(EngineSettings& settings) {
    VkAttachmentDescription colourAttachment = {};
    {
        colourAttachment.format = this->swapchainImageFormat;
        colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

        colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    VkAttachmentReference colourAttachmentReference = {};
    {
        colourAttachmentReference.attachment = 0;
        colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDependency colourSubpassDependency = {};
    {
        colourSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        colourSubpassDependency.dstSubpass = 0;
        colourSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        colourSubpassDependency.srcAccessMask = 0;
        colourSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        colourSubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    }

    VkSubpassDescription subpass = {};
    {
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colourAttachmentReference;
        subpass.pDepthStencilAttachment = nullptr;
    }

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    {
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &colourAttachment;

        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;

        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &colourSubpassDependency;
    }

    if (vkCreateRenderPass(this->device, &renderPassCreateInfo, nullptr, &combinationRenderpass) != VK_SUCCESS) {
        Logger::error("Failed to create combination renderpass");
        return false;
    }

    return true;
}



bool VulkanRenderer::initCombinationFramebuffers(EngineSettings& settings) {
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;

    framebufferCreateInfo.renderPass = this->combinationRenderpass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.width = settings.windowWidth;
    framebufferCreateInfo.height = settings.windowHeight;
    framebufferCreateInfo.layers = 1;

    for (SwapchainData& swapchain : this->swapchainData) {
        framebufferCreateInfo.pAttachments = &swapchain.swapchainImageView;
        if (vkCreateFramebuffer(this->device, &framebufferCreateInfo, nullptr, &swapchain.framebuffer) != VK_SUCCESS) {
            Logger::error("Failed to create combination framebuffer");
            return false;
        }
    }

    return true;
}

/* ========================================= */
/* Cleanup                                   */
/* ========================================= */

void VulkanRenderer::cleanupSwapchain() {
    vkDeviceWaitIdle(this->device);

    for (auto& data : swapchainData) {
        vkDestroyImageView(this->device, data.depthImageView, nullptr);
        vmaDestroyImage(this->allocator, data.depthImage.image, data.depthImage.allocation);

        vkDestroyFramebuffer(this->device, data.framebuffer, nullptr);
        vkDestroyImageView(this->device, data.swapchainImageView, nullptr);
    }

    vkDestroySwapchainKHR(this->device, this->swapchainHandle, nullptr);
}

void VulkanRenderer::cleanup() {
    vkDeviceWaitIdle(this->device);

    cleanupSwapchain();

    for (auto& allocatedBuffer: bufferList.getMap()) {
        vmaDestroyBuffer(this->allocator, allocatedBuffer.second.buffer, allocatedBuffer.second.allocation);
    }
    bufferList.clear();

    for (auto& vMat: materialList.getMap()) {
        vMat.second.deleteMaterial(device);
    }
    materialList.clear();

    for (const auto& image: imageList.getMap()) {
        if (image.second.sampler != VK_NULL_HANDLE) vkDestroySampler(this->device, image.second.sampler, nullptr);
        if (image.second.imageView != VK_NULL_HANDLE) vkDestroyImageView(this->device, image.second.imageView, nullptr);
        vmaDestroyImage(this->allocator, image.second.image.image, image.second.image.allocation);
    }

    deletionQueue.flush(this->device);

    vmaDestroyBuffer(this->allocator, sceneParamsBuffer.buffer, sceneParamsBuffer.allocation);

    vkDestroyRenderPass(this->device, this->combinationRenderpass, nullptr);

    // Cleanup frameData
    for (FrameData& data : this->frameData) {
        vmaDestroyBuffer(this->allocator, data.cameraBuffer.buffer, data.cameraBuffer.allocation);
        vmaDestroyBuffer(this->allocator, data.objectBuffer.buffer, data.objectBuffer.allocation);

        vkDestroyFence(this->device, data.renderFence, nullptr);
        vkDestroySemaphore(this->device, data.renderSemaphore, nullptr);
        vkDestroySemaphore(this->device, data.presentSemaphore, nullptr);

        vkDestroyCommandPool(this->device, data.commandPool, nullptr);
    }

    vkDestroyDescriptorPool(this->device, this->descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(this->device, this->globalDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(this->device, this->deferredPassDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(this->device, this->combinationPassDescriptorSetLayout, nullptr);

    vmaDestroyAllocator(this->allocator);

    vkDestroyDevice(this->device, nullptr);
    vkDestroySurfaceKHR(this->vInstance, this->surface, nullptr);

    vkb::destroy_debug_utils_messenger(this->vInstance, this->debugMessenger);
    vkDestroyInstance(this->vInstance, nullptr);

    Renderer::cleanup();
}

/* ========================================= */
/* Rendering                                 */
/* ========================================= */

void VulkanRenderer::drawFrame(const double deltaTime, const double gameTime, const Scene& scene) {
    std::vector<const Actor*> toRender;
    std::vector<const Actor*> toRenderCollision;
    // Calculate actors to render
    {
        for (const Actor* actor : scene.actors) {
            if (actor->hasMesh()) toRender.push_back(actor);
            if (actor->hasCollision()) toRenderCollision.push_back(actor);
        }
    }

    FrameData& frame = getCurrentFrame();

    vkWaitForFences(this->device, 1, &frame.renderFence, true, UINT64_MAX);
    vkResetFences(this->device, 1, &frame.renderFence);

    uint32_t swapchainIndex;
    vkAcquireNextImageKHR(this->device, this->swapchainHandle, UINT64_MAX, frame.presentSemaphore, VK_NULL_HANDLE, &swapchainIndex);

    SwapchainData swapchain = swapchainData[swapchainIndex];

    uint32_t swapchainUniformOffset = padUniformBufferSize(sizeof(GPUSceneData)) * swapchainIndex;

    // Setup global descriptor sets
    {
        // Camera Buffer
        {
            glm::vec3 camPos = {0.f, 0.f, -10.f};
            const glm::mat4& viewMat = glm::translate(glm::mat4(1.f), camPos) *
                                       glm::mat4_cast(scene.controlledActor->getRotation());
            GpuCameraData cameraData = {
                    {
                            viewMat,
                            glm::perspective(glm::radians(90.f),
                                             ((float) settings->windowWidth) / ((float) settings->windowHeight), 0.1f,
                                             200.f)
                    },
                    {
                            // TODO: The viewMat can be handled better
                            (glm::vec3) (viewMat * glm::vec4(1))
                    }
            };

            void* data;
            vmaMapMemory(this->allocator, frame.cameraBuffer.allocation, &data);
            memcpy(data, &cameraData, sizeof(GpuCameraData));
            vmaUnmapMemory(this->allocator, frame.cameraBuffer.allocation);
        }

        // Scene Buffer
        {
            float framed = (this->currentFrame / 1200.f);

            sceneParams.ambientColor = {std::sin(framed), 0, std::cos(framed), 1};
            char* sceneData;
            vmaMapMemory(allocator, sceneParamsBuffer.allocation, (void**) &sceneData);
            sceneData += swapchainUniformOffset;
            memcpy(sceneData, &sceneParams, sizeof(GPUSceneData));
            vmaUnmapMemory(allocator, sceneParamsBuffer.allocation);
        }
    }

    // Deferred Renderpass
    {
        vkResetCommandBuffer(frame.deferredCommandBuffer, 0);

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = nullptr;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;

        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(frame.deferredCommandBuffer, &commandBufferBeginInfo);

        VkClearValue clearValues[5];
        float flash = (float) std::abs(std::sin(gameTime));
        clearValues[0].color = {{0.f, 0.f, 0.f, 1.f}};
        clearValues[1].color = {{0.f, 0.f, flash, 1.f}};
        clearValues[2].color = {{0.f, 0.f, 0.f, 1.f}};
        clearValues[3].color = {{0.f, 0.f, 0.f, 1.f}};

        clearValues[4].depthStencil.depth = 1.f;
        clearValues[4].depthStencil.stencil = 0;

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext = nullptr;

        renderPassBeginInfo.renderPass = this->deferredRenderpass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = {settings->windowWidth, settings->windowHeight};
        renderPassBeginInfo.framebuffer = deferredFrameData.framebuffer;

        renderPassBeginInfo.clearValueCount = 5;
        renderPassBeginInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(frame.deferredCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Deferred descriptor set
        {
            void* objectData;
            vmaMapMemory(this->allocator, frame.objectBuffer.allocation, &objectData);
            GpuObjectData* objectSSBO = (GpuObjectData*) objectData;

            for (int i = 0; i < toRender.size(); ++i) {
                const Actor* actor = toRender[i];
                glm::mat4 model = glm::translate(glm::mat4(1.f), actor->getPosition());
                model = glm::scale(model, actor->getScale());
                model = model * glm::mat4_cast(actor->getRotation());
                objectSSBO[i].modelMatrix = model;
            }

            for (int i = 0; i < toRenderCollision.size(); ++i) {
                const Actor* actor = toRenderCollision[i];

                glm::mat4 model = glm::translate(actor->actorCollider->getRenderMeshTransform(), actor->getPosition());
                objectSSBO[i + toRender.size()].modelMatrix = model;
            }

            vmaUnmapMemory(allocator, frame.objectBuffer.allocation);
        }

        uint64_t prevVMat = 0;
        for (int i = 0; i < toRender.size(); ++i) {
            const Actor* actor = toRender[i];
            StaticMesh& mesh = *actor->actorMesh;
            VMaterial vMat = materialList.get(mesh.material->materialId);
            if (prevVMat != mesh.material->materialId) {
                prevVMat = mesh.material->materialId;

                vkCmdBindPipeline(frame.deferredCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipeline);

                vkCmdBindDescriptorSets(frame.deferredCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipelineLayout,
                                        0, 1, &frame.globalDescriptor, 1, &swapchainUniformOffset);
                vkCmdBindDescriptorSets(frame.deferredCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipelineLayout,
                                        1, 1, &frame.deferredPassDescriptor, 0, nullptr);
                vkCmdBindDescriptorSets(frame.deferredCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipelineLayout,
                                        2, 1, &vMat.materialDescriptor, 0, nullptr);
            }

            AllocatedBuffer vertBuffer = this->bufferList.get(mesh.mesh->verticesId);
            AllocatedBuffer indBuffer = this->bufferList.get(mesh.mesh->indicesId);

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(frame.deferredCommandBuffer, 0, 1, &vertBuffer.buffer, &offset);
            vkCmdBindIndexBuffer(frame.deferredCommandBuffer, indBuffer.buffer, offset, VkIndexType::VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(frame.deferredCommandBuffer, mesh.mesh->indices.size(), 1, 0, 0, i);
        }

        VMaterial vMat = materialList.get(collisionMat.materialId);
        vkCmdBindPipeline(frame.deferredCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipeline);

        uint32_t uniformOffset = padUniformBufferSize(sizeof(GPUSceneData)) * swapchainIndex;
        vkCmdBindDescriptorSets(frame.deferredCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipelineLayout,
                                0, 1, &frame.globalDescriptor, 1, &uniformOffset);

        for (int i = 0; i < toRenderCollision.size(); ++i) {
            const Actor* actor = toRenderCollision[i];
            Mesh* mesh = actor->actorCollider->getRenderMesh();

            AllocatedBuffer vertBuffer = this->bufferList.get(mesh->verticesId);
            AllocatedBuffer indBuffer = this->bufferList.get(mesh->indicesId);

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(frame.deferredCommandBuffer, 0, 1, &vertBuffer.buffer, &offset);
            vkCmdBindIndexBuffer(frame.deferredCommandBuffer, indBuffer.buffer, offset, VkIndexType::VK_INDEX_TYPE_UINT32);

            MeshPushConstants pushConstants = {
                    {},
                    actor->actorCollider->isColliding
                        ? glm::vec4(0.f, 1.f, 0.f, 1.f)
                        : glm::vec4(1.f, 0.f, 0.f, 1.f)
            };
            vkCmdPushConstants(frame.deferredCommandBuffer, vMat.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                               sizeof(pushConstants), &pushConstants);

            vkCmdDrawIndexed(frame.deferredCommandBuffer, mesh->indices.size(), 1, 0, 0, i + toRender.size());
        }

        vkCmdEndRenderPass(frame.deferredCommandBuffer);
        vkEndCommandBuffer(frame.deferredCommandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        submitInfo.pWaitDstStageMask = &waitStage;

        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &frame.presentSemaphore;

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &deferredFrameData.deferredSemaphore;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &frame.deferredCommandBuffer;

        vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    }

    // Combination Renderpass
    {
        vkResetCommandBuffer(frame.combinationCommandBuffer, 0);

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = nullptr;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;

        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(frame.combinationCommandBuffer, &commandBufferBeginInfo);

        VkClearValue clearValues[1];
        clearValues[0].color = {{0.f, 0.f, 0.f, 1.f}};

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext = nullptr;

        renderPassBeginInfo.renderPass = this->combinationRenderpass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = {settings->windowWidth, settings->windowHeight};
        renderPassBeginInfo.framebuffer = swapchain.framebuffer;

        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(frame.combinationCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Bind pipeline and descriptors
        {
            VMaterial vMat = materialList.get(combinationMat.materialId);
            vkCmdBindPipeline(frame.combinationCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipeline);

            vkCmdBindDescriptorSets(frame.combinationCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipelineLayout,
                                    0, 1, &frame.globalDescriptor, 1, &swapchainUniformOffset);
            vkCmdBindDescriptorSets(frame.combinationCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipelineLayout,
                                    1, 1, &frame.combinationPassDescriptor, 0, nullptr);

        }

        vkCmdDraw(frame.combinationCommandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(frame.combinationCommandBuffer);
        vkEndCommandBuffer(frame.combinationCommandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        submitInfo.pWaitDstStageMask = &waitStage;

        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &deferredFrameData.deferredSemaphore;

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &frame.renderSemaphore;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &frame.combinationCommandBuffer;

        vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, frame.renderFence);
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;

    presentInfo.pSwapchains = &this->swapchainHandle;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &frame.renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapchainIndex;

    vkQueuePresentKHR(this->graphicsQueue, &presentInfo);
    ++currentFrame;
}

/* ========================================= */
/* Internal Data Management                  */
/* ========================================= */

VkResult VulkanRenderer::executeTransfer(std::function<VkResult(VkCommandBuffer)>&& function) {
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    // Command Buffer
    VkResult result;
    {
        vkBeginCommandBuffer(transferContext.commandBuffer, &commandBufferBeginInfo);
        result = function(transferContext.commandBuffer);
        vkEndCommandBuffer(transferContext.commandBuffer);
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;

    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    submitInfo.pWaitDstStageMask = nullptr;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &transferContext.commandBuffer;

    vkQueueSubmit(transferQueue, 1, &submitInfo, transferContext.transferFence);

    vkWaitForFences(this->device, 1, &transferContext.transferFence, true, UINT64_MAX);
    vkResetFences(this->device, 1, &transferContext.transferFence);

    vkResetCommandPool(this->device, transferContext.commandPool, 0);

    return result;
}

VkResult VulkanRenderer::executeTransfer(std::function<VkResult(VkCommandBuffer, VkCommandBuffer)>&& function) {
        // Command Buffer
    VkResult result;
    {
        result = function(transferContext.commandBuffer, globalGraphicsContext.commandBuffer);
    }
    VkSubmitInfo transferSubmitInfo {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,

        0,
        nullptr,
        nullptr,
        1,
        &transferContext.commandBuffer,
        1,
        &transferContext.transferSemaphore
    };
    vkQueueSubmit(transferQueue, 1, &transferSubmitInfo, VK_NULL_HANDLE);

    VkSubmitInfo graphicsSubmitInfo {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,

        1,
        &transferContext.transferSemaphore,
        nullptr,
        1,
        &globalGraphicsContext.commandBuffer,
        0,
        nullptr
    };
    vkQueueSubmit(graphicsQueue, 1, &graphicsSubmitInfo, transferContext.transferFence);

    vkWaitForFences(this->device, 1, &transferContext.transferFence, true, UINT64_MAX);
    vkResetFences(this->device, 1, &transferContext.transferFence);

    vkResetCommandPool(this->device, transferContext.commandPool, 0);
    vkResetCommandPool(this->device, globalGraphicsContext.commandPool, 0);

    return result;
}

bool VulkanRenderer::loadShader(const std::string& path, VkShaderModule* outShaderModule) {
    std::ifstream shaderFile(path, std::ios::ate | std::ios::binary);

    if (!shaderFile.is_open()) {
        Logger::error("Failed to open file: " + path);
        return false;
    }

    size_t fileSize = shaderFile.tellg();

    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    shaderFile.seekg(0);
    shaderFile.read((char*) buffer.data(), fileSize);

    VkShaderModuleCreateInfo shaderModuleCreateInfo ={};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = nullptr;

    shaderModuleCreateInfo.codeSize = fileSize;
    shaderModuleCreateInfo.pCode = buffer.data();

    if (vkCreateShaderModule(this->device, &shaderModuleCreateInfo, nullptr, outShaderModule)) {
        Logger::warn("Failed to create shader module");
        return false;
    }

    return true;
}

AllocatedBuffer VulkanRenderer::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) {
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;

    bufferCreateInfo.size = allocSize;
    bufferCreateInfo.usage = usage;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.flags = flags;
    allocationCreateInfo.usage = memoryUsage;

    AllocatedBuffer allocatedBuffer;

    if (vmaCreateBuffer(this->allocator, &bufferCreateInfo, &allocationCreateInfo, &allocatedBuffer.buffer, &allocatedBuffer.allocation, nullptr) != VK_SUCCESS) {
        Logger::warn("Failed to allocate buffer");
        return {VK_NULL_HANDLE, VK_NULL_HANDLE};
    }

    return allocatedBuffer;
}

void VulkanRenderer::uploadMesh(Mesh& mesh) {
    const size_t verticesSize = mesh.vertices.size() * sizeof(Vertex);
    const size_t indicesSize = mesh.indices.size() * sizeof(uint32_t);

    AllocatedBuffer stagingBuffer = this->createBuffer(
            verticesSize + indicesSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );

    // Transfer data to staging buffer
    void* data;
    vmaMapMemory(this->allocator, stagingBuffer.allocation, &data);
    memcpy(data, mesh.vertices.data(), verticesSize);
    memcpy((char*) data + verticesSize, mesh.indices.data(), indicesSize);  // Dirty pointer arithmetic
    vmaUnmapMemory(this->allocator, stagingBuffer.allocation);

    AllocatedBuffer vertexBuffer = this->createBuffer(
            verticesSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
    );
    AllocatedBuffer indexBuffer = this->createBuffer(
            indicesSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
    );

    mesh.verticesId = this->bufferList.insert(vertexBuffer);
    mesh.indicesId = this->bufferList.insert(indexBuffer);

    this->executeTransfer([&](VkCommandBuffer commandBuffer) {
        VkBufferCopy copy{};
        copy.dstOffset = 0;

        // Copy Vertices
        copy.srcOffset = 0;
        copy.size = verticesSize;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer.buffer, vertexBuffer.buffer, 1, &copy);

        // Copy Indices
        copy.srcOffset = verticesSize;
        copy.size = indicesSize;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer.buffer, indexBuffer.buffer, 1, &copy);

        return VK_SUCCESS;
    });

    vmaDestroyBuffer(this->allocator, stagingBuffer.buffer, stagingBuffer.allocation);
}

void VulkanRenderer::uploadTexture(Texture& texture) {
    size_t bufferSize = texture.size();
    AllocatedBuffer stagingBuffer = this->createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );

    // Transfer data to staging buffer
    void* data;
    vmaMapMemory(this->allocator, stagingBuffer.allocation, &data);
    memcpy(data, texture.pixels.data(), bufferSize);
    vmaFlushAllocation(allocator, stagingBuffer.allocation, 0, bufferSize);
    vmaUnmapMemory(this->allocator, stagingBuffer.allocation);

    VkFormat format = textureFormatToVkFormat(texture.format);
    VkExtent3D textureExtent{
            (uint32_t) texture.width,
            (uint32_t) texture.height,
            1
    };

    VTexture vTexture;
    VKShortcuts::createAllocatedImage(this->allocator,
                                      format,
                                      textureExtent,
                                      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                      0,
                                      vTexture.image);
    VKShortcuts::createImageView(this->device,
                                 format,
                                 vTexture.image.image,
                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                 vTexture.imageView);
    
    {
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.pNext = nullptr;
        samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
        samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        vkCreateSampler(device, &samplerCreateInfo, nullptr, &vTexture.sampler);
    }
    texture.textureId = imageList.insert(vTexture);

    // Transfer data to real buffer
    executeTransfer([&](VkCommandBuffer transferCommandBuffer, VkCommandBuffer graphicsCommandBuffer) {
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = nullptr;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(transferCommandBuffer, &commandBufferBeginInfo);
        VkImageSubresourceRange range{
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            1,
            0,
            1
        };

        VkImageMemoryBarrier transferBarrier {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            0,
            VK_ACCESS_MEMORY_WRITE_BIT,

            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            vTexture.image.image,
            range
        };

        vkCmdPipelineBarrier(transferCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &transferBarrier);

        VkBufferImageCopy copy{
            0,
            0,
            0,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                0,
                1
            },
            {},
            textureExtent
        };

        vkCmdCopyBufferToImage(transferCommandBuffer, stagingBuffer.buffer, vTexture.image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

        VkImageMemoryBarrier readableBarrierTransfer {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                nullptr,
                VK_ACCESS_MEMORY_WRITE_BIT,
                VK_ACCESS_SHADER_READ_BIT,

                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                transferQueueIndex,
                graphicsQueueIndex,
                vTexture.image.image,
                range
        };

        vkCmdPipelineBarrier(transferCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &readableBarrierTransfer);
        vkEndCommandBuffer(transferCommandBuffer);

        vkBeginCommandBuffer(graphicsCommandBuffer, &commandBufferBeginInfo);

        VkImageMemoryBarrier readableBarrierGraphics {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                nullptr,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_ACCESS_SHADER_READ_BIT,

                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                transferQueueIndex,
                graphicsQueueIndex,
                vTexture.image.image,
                range
        };

        vkCmdPipelineBarrier(graphicsCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &readableBarrierGraphics);
        vkEndCommandBuffer(graphicsCommandBuffer);
        return VK_SUCCESS;
    });

    vmaDestroyBuffer(this->allocator, stagingBuffer.buffer, stagingBuffer.allocation);
}


bool VulkanRenderer::createMaterial(Material& material) {
    VkDescriptorSetLayout materialLayout;
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (int i = 0; i < material.textures.size(); ++i) {
            Texture* texture = material.textures[i];

            registerTexture(texture);

            bindings.push_back(VKShortcuts::createDescriptorSetLayoutBinding(i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT));
        }
        VkDescriptorSetLayoutCreateInfo materialLayoutCreateInfo{
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(bindings.size()),
            bindings.data()
        };

        vkCreateDescriptorSetLayout(this->device, &materialLayoutCreateInfo, nullptr, &materialLayout);
    }

    VkPipelineLayout pipelineLayout;
    {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.pNext = nullptr;

        pipelineLayoutCreateInfo.flags = 0;

        VkDescriptorSetLayout passLayout;
        switch (material.shaderType) {

            case ShaderType::OPAQUE:
            case ShaderType::WIREFRAME:
            case ShaderType::TRANSPARENT:
                passLayout = this->deferredPassDescriptorSetLayout;
                break;
            case ShaderType::COMBINATION:
                passLayout = this->combinationPassDescriptorSetLayout;
                break;
        }

        VkDescriptorSetLayout setLayouts[] = {this->globalDescriptorSetLayout, passLayout, materialLayout};

        pipelineLayoutCreateInfo.setLayoutCount = 3;
        pipelineLayoutCreateInfo.pSetLayouts = setLayouts;

        VkPushConstantRange pushConstantRange;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(MeshPushConstants);
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(this->device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            Logger::error("Failed to create pipeline layout");
            return false;
        }
    }

    VkRenderPass renderpass;
    switch (material.shaderType) {
        case ShaderType::OPAQUE:
        case ShaderType::WIREFRAME:
            renderpass = this->deferredRenderpass;
            break;
        case ShaderType::COMBINATION:
            renderpass = this->combinationRenderpass;
            break;
        case ShaderType::TRANSPARENT:
            // TODO: Implement
            renderpass = VK_NULL_HANDLE;
            break;
    }

    PipelineBuilder builder = PipelineBuilder(this->device,
                                              renderpass,
                                              pipelineLayout)
            .setInputAssemblyInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .setViewport(0.f, 0.f, (float) this->settings->windowWidth, (float) this->settings->windowHeight)
            .setScissor(0, 0, this->settings->windowWidth, this->settings->windowHeight)
            .setRasterisationState(material.shaderType == ShaderType::WIREFRAME ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL)
            .setMultisampleStateDefault()
            .setDepthStencilState(true, true, VK_COMPARE_OP_LESS, false);

    if (material.shaderType == ShaderType::COMBINATION) {
        builder
                .setVertexInputInfoDefault()
                .addColourBlendAttachmentDefaultNoBlend();
    } else {
        VertexDescription vertexDescription = VertexDescription::getVertexDescription();
        builder
                .setVertexInputInfo(vertexDescription)
                .addColourBlendAttachmentDefaultNoBlend()
                .addColourBlendAttachmentDefaultNoBlend()
                .addColourBlendAttachmentDefaultNoBlend();
    }

    std::vector<VkShaderModule> shaderModules;
    shaderModules.reserve(material.materialStages.size());
    for (const auto& stage: material.materialStages) {
        VkShaderModule shaderModule;
        if (!this->loadShader(FileUtils::getAssetsPath() + stage.shaderPathSpirv, &shaderModule)) {
            Logger::error("Failed to open shader: " + stage.shaderPathSpirv);
            return false;
        }

        builder.addShaderStage(MaterialUtil::stageFlagFromShaderStage(stage.shaderStage), shaderModule);
        shaderModules.push_back(shaderModule);
    }

    std::optional<VkPipeline> pipeline = builder.buildPipeline();

    if (!pipeline.has_value()) {
        Logger::error("Failed to create pipeline");
        return false;
    }

    VkDescriptorSet materialSet;
    {
        VkDescriptorSetAllocateInfo materialDescriptorSetAllocInfo{
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                nullptr,
                this->descriptorPool,
                1,
                &materialLayout
        };
        vkAllocateDescriptorSets(this->device, &materialDescriptorSetAllocInfo, &materialSet);

        std::vector<VkWriteDescriptorSet> writeDescriptorSets(material.textures.size());

        VkDescriptorImageInfo imageInfos[material.textures.size()];
        for (int i = 0; i < material.textures.size(); ++i) {
            Texture* texture = material.textures[i];
            VTexture vTexture = imageList.get(texture->textureId);

            // Workaround so memory doesn't get freed outside the loop
            imageInfos[i] = {
                vTexture.sampler,
                vTexture.imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };

            writeDescriptorSets[i] = VKShortcuts::createWriteDescriptorSetImage(i, materialSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageInfos + i);
        }

        vkUpdateDescriptorSets(this->device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);

    }
    material.materialId = this->materialList.insert(VMaterial(pipeline.value(), pipelineLayout, materialLayout, materialSet));

    for (const auto& shaderModule: shaderModules) {
        vkDestroyShaderModule(this->device, shaderModule, nullptr);
    }

    return true;
}

/* ========================================= */
/* Resource Management                       */
/* ========================================= */

void VulkanRenderer::setupScene(Scene& scene) {
    for (const Actor* actor : scene.actors) {
        if (actor->hasMesh()) {
            // Upload mesh
            registerMesh(actor->actorMesh->mesh);

            // Upload Material
            createMaterial(*actor->actorMesh->material);
        }
        if (actor->hasCollision()) {
            // Upload collision mesh
            // TODO: Add flag for this
            registerMesh(actor->actorCollider->getRenderMesh());
        }
    }
}

bool VulkanRenderer::registerMesh(Mesh* mesh) {
    if (mesh->verticesId && mesh->indicesId) return false;

    uploadMesh(*mesh);

    return true;
}



bool VulkanRenderer::registerTexture(Texture* texture) {
    if (texture->textureId) return false;

    uploadTexture(*texture);

    return true;
}


bool VulkanRenderer::registerMaterial(Material* material) {
    if (material->materialId) return false;

    createMaterial(*material);

    return true;
}

/* ========================================= */
/* Util                                      */
/* ========================================= */

FrameData& VulkanRenderer::getCurrentFrame() {
    return frameData[currentFrame % settings->bufferCount];
}

size_t VulkanRenderer::padUniformBufferSize(size_t originalSize) {
    // Calculate required alignment based on minimum device offset alignment
    size_t minUboAlignment = gpuProperties.limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = originalSize;
    if (minUboAlignment > 0) {
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    return alignedSize;
}
