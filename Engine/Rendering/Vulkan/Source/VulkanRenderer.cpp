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
#include "Rendering/GPUStructures/GpuCameraStruct.h"
#include <Collision/SphereCollider.h>

bool VulkanRenderer::initialise(EngineSettings& settings) {
    Renderer::initialise(settings);

    if (!this->initVulkan(settings)) return false;
    if (!this->initSwapchain(settings)) return false;
    if (!this->initRenderpass(settings)) return false;
    if (!this->initFramebuffers(settings)) return false;

    // Initialise collision visualisation
    createMaterial(this->collisionMat);

    return true;
}

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

VulkanRenderer::~VulkanRenderer() = default;

void VulkanRenderer::setupGLFWHints() {
    Renderer::setupGLFWHints();
    // Since we're using vulkan, tell it not to create an opengl context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Prevent resizing the window
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
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

        auto vkbDev = vkb::DeviceBuilder(vkbPDevice)
                .build();

        if (!deviceSelector.has_value()) {
            Logger::error("Failed to create logical device");
            return false;
        }

        vkbDevice = vkbDev.value();

        this->device = vkbDevice.device;
        this->gpu = vkbDevice.physical_device;

        this->graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
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

    this->initDescriptors(settings);

    return this->initFrameData(settings, vkbDevice.get_queue_index(vkb::QueueType::graphics).value());
}

void VulkanRenderer::initDescriptors(EngineSettings& settings) {
    VkDescriptorSetLayoutBinding cameraBufferBinding = {};
    cameraBufferBinding.binding = 0;
    cameraBufferBinding.descriptorCount = 1;
    cameraBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    cameraBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.pNext = nullptr;
    descriptorSetLayoutCreateInfo.flags = 0;

    descriptorSetLayoutCreateInfo.bindingCount = 1;
    descriptorSetLayoutCreateInfo.pBindings = &cameraBufferBinding;
    vkCreateDescriptorSetLayout(this->device, &descriptorSetLayoutCreateInfo, nullptr, &this->globalDescriptorSetLayout);

    std::vector<VkDescriptorPoolSize> sizes = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10}
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.pNext = nullptr;

    descriptorPoolCreateInfo.flags = 0;
    descriptorPoolCreateInfo.maxSets = 10;
    descriptorPoolCreateInfo.poolSizeCount = (uint32_t) sizes.size();
    descriptorPoolCreateInfo.pPoolSizes = sizes.data();

    vkCreateDescriptorPool(this->device, &descriptorPoolCreateInfo, nullptr, &this->descriptorPool);
}

bool VulkanRenderer::initFrameData(EngineSettings& settings, unsigned int graphicsQueueIndex) {
    frameData.resize(settings.bufferCount);

    for (auto& data : frameData) {
        if (!initFrameDataGraphicsPools(settings, data, graphicsQueueIndex)) return false;

        if (!initFrameDataSyncObjects(settings, data)) return false;

        initFrameDataDescriptorSets(settings, data);
    }
    return true;
}

bool VulkanRenderer::initFrameDataGraphicsPools(EngineSettings& settings, FrameData& frameData, unsigned int graphicsQueueIndex) {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;

    commandPoolCreateInfo.queueFamilyIndex = graphicsQueueIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;

    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkCreateCommandPool(this->device, &commandPoolCreateInfo, nullptr, &frameData.commandPool) != VK_SUCCESS) {
        Logger::error("Failed to create command pool");
        return false;
    }

    commandBufferAllocateInfo.commandPool = frameData.commandPool;
    if (vkAllocateCommandBuffers(this->device, &commandBufferAllocateInfo, &frameData.commandBuffer) != VK_SUCCESS) {
        Logger::error("Failed to create command buffer");
        return false;
    }

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
            Logger::error("Failed to create fence");
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
            Logger::error("Failed to create semaphore");
            return false;
        }
    }
    return true;
}

void VulkanRenderer::initFrameDataDescriptorSets(EngineSettings& settings, FrameData& frameData) {
    frameData.cameraBuffer = createBuffer(sizeof(GpuCameraStruct), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.pNext = nullptr;

    descriptorSetAllocateInfo.descriptorPool = this->descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &this->globalDescriptorSetLayout;

    vkAllocateDescriptorSets(this->device, &descriptorSetAllocateInfo, &frameData.globalDescriptor);

    VkDescriptorBufferInfo descriptorBufferInfo;
    descriptorBufferInfo.buffer = frameData.cameraBuffer.buffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = sizeof(GpuCameraStruct);

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;

    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstSet = frameData.globalDescriptor;

    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
    vkUpdateDescriptorSets(this->device, 1, &writeDescriptorSet, 0, nullptr);
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

        if (!this->initSwapchainDepthBuffer(settings, this->swapchainData[i])) return false;
    }
    return true;
}

bool VulkanRenderer::initSwapchainDepthBuffer(EngineSettings& settings, SwapchainData& swapchain) {
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = this->depthFormat;
    imageCreateInfo.extent = {settings.windowWidth, settings.windowHeight, 1};
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocationCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = nullptr;

    imageViewCreateInfo.format = this->depthFormat;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    vmaCreateImage(this->allocator, &imageCreateInfo, &allocationCreateInfo, &swapchain.depthImage.image,
                   &swapchain.depthImage.allocation, nullptr);

    imageViewCreateInfo.image = swapchain.depthImage.image;
    if (vkCreateImageView(this->device, &imageViewCreateInfo, nullptr, &swapchain.depthImageView) != VK_SUCCESS) {
        Logger::error("Failed to create depth imageview");
        return false;
    }
    return true;
}

bool VulkanRenderer::initFramebuffers(EngineSettings& settings) {
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;

    framebufferCreateInfo.renderPass = this->renderPass;
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.width = settings.windowWidth;
    framebufferCreateInfo.height = settings.windowHeight;
    framebufferCreateInfo.layers = 1;

    for (SwapchainData& swapchain : this->swapchainData) {
        VkImageView attachments[2] = {swapchain.swapchainImageView, swapchain.depthImageView};
        framebufferCreateInfo.pAttachments = attachments;
        if (vkCreateFramebuffer(this->device, &framebufferCreateInfo, nullptr, &swapchain.framebuffer) != VK_SUCCESS) {
            Logger::error("Failed to create framebuffer");
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::initRenderpass(EngineSettings& settings) {
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

    VkAttachmentDescription depthAttachment = {};
    {
        depthAttachment.flags = 0;

        depthAttachment.format = this->depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentReference depthAttachmentReference = {};
    {
        depthAttachmentReference.attachment = 1;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDependency depthSubpassDependency = {};
    {
        depthSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        depthSubpassDependency.dstSubpass = 0;
        depthSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthSubpassDependency.srcAccessMask = 0;
        depthSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthSubpassDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    VkSubpassDescription subpass = {};
    {
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colourAttachmentReference;
        subpass.pDepthStencilAttachment = &depthAttachmentReference;
    }

    VkAttachmentDescription attachments[2] = {colourAttachment, depthAttachment};
    VkSubpassDependency dependencies[2] = {colourSubpassDependency, depthSubpassDependency};
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    {
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        renderPassCreateInfo.attachmentCount = 2;
        renderPassCreateInfo.pAttachments = attachments;

        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;

        renderPassCreateInfo.dependencyCount = 2;
        renderPassCreateInfo.pDependencies = dependencies;
    }

    if (vkCreateRenderPass(this->device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
        Logger::error("Failed to create renderpass");
        return false;
    }

    return true;
}

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
    bufferList.clear();

    deletionQueue.flush(this->device);

    vkDestroyRenderPass(this->device, this->renderPass, nullptr);

    // Cleanup frameData
    for (FrameData& data : this->frameData) {
        vmaDestroyBuffer(this->allocator, data.cameraBuffer.buffer, data.cameraBuffer.allocation);

        vkDestroyFence(this->device, data.renderFence, nullptr);
        vkDestroySemaphore(this->device, data.renderSemaphore, nullptr);
        vkDestroySemaphore(this->device, data.presentSemaphore, nullptr);

        vkDestroyCommandPool(this->device, data.commandPool, nullptr);
    }

    vkDestroyDescriptorPool(this->device, this->descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(this->device, this->globalDescriptorSetLayout, nullptr);

    vmaDestroyAllocator(this->allocator);

    vkDestroyDevice(this->device, nullptr);
    vkDestroySurfaceKHR(this->vInstance, this->surface, nullptr);

    vkb::destroy_debug_utils_messenger(this->vInstance, this->debugMessenger);
    vkDestroyInstance(this->vInstance, nullptr);

    Renderer::cleanup();
}

void VulkanRenderer::drawFrame(const double deltaTime, const double gameTime, const Scene& scene) {
    FrameData& frame = getCurrentFrame();

    vkWaitForFences(this->device, 1, &frame.renderFence, true, UINT64_MAX);
    vkResetFences(this->device, 1, &frame.renderFence);

    uint32_t swapchainIndex;
    vkAcquireNextImageKHR(this->device, this->swapchainHandle, UINT64_MAX, frame.presentSemaphore, VK_NULL_HANDLE, &swapchainIndex);

    SwapchainData swapchain = swapchainData[swapchainIndex];

    vkResetCommandBuffer(frame.commandBuffer, 0);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(frame.commandBuffer, &commandBufferBeginInfo);

    VkClearValue clearValues[2];
    float flash = (float) std::abs(std::sin(gameTime));
    clearValues[0].color = {{0.f, 0.f, flash, 1.f}};

    clearValues[1].depthStencil.depth = 1.f;
    clearValues[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;

    renderPassBeginInfo.renderPass = this->renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = {1366, 768};
    renderPassBeginInfo.framebuffer = swapchain.framebuffer;

    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    // Renderpass
    {
        vkCmdBeginRenderPass(frame.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        glm::vec3 camPos = {0.f, 0.f, -10.f};
        GpuCameraStruct cameraData = {
                glm::translate(glm::mat4(1.f), camPos),
                glm::perspective(glm::radians(90.f), 1366.f/768.f, 0.1f, 200.f)
        };

        void* data;
        vmaMapMemory(this->allocator, frame.cameraBuffer.allocation, &data);
        memcpy(data, &cameraData, sizeof(GpuCameraStruct));
        vmaUnmapMemory(this->allocator, frame.cameraBuffer.allocation);

        uint64_t prevVMat = 0;
        for (const Actor* actor : scene.actors) {
            if (actor->hasMesh()) {
                StaticMesh& mesh = *actor->actorMesh;
                VMaterial vMat = materialList.get(mesh.material->materialId);
                if (prevVMat != mesh.material->materialId) {
                    prevVMat = mesh.material->materialId;

                    vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipeline);
                    vkCmdBindDescriptorSets(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipelineLayout,
                                            0, 1, &frame.globalDescriptor, 0, nullptr);
                }

                AllocatedBuffer vertBuffer = this->bufferList.get(mesh.mesh->verticesId);
                AllocatedBuffer indBuffer = this->bufferList.get(mesh.mesh->indicesId);

                VkDeviceSize offset = 0;
                vkCmdBindVertexBuffers(frame.commandBuffer, 0, 1, &vertBuffer.buffer, &offset);
                vkCmdBindIndexBuffer(frame.commandBuffer, indBuffer.buffer, offset, VkIndexType::VK_INDEX_TYPE_UINT32);

                glm::mat4 model = glm::translate(glm::mat4(1.f), actor->getPosition());
                model = glm::scale(model, actor->getScale());
                model = model * glm::mat4_cast(actor->getRotation());

                MeshPushConstants pushConstants = {model};
                vkCmdPushConstants(frame.commandBuffer, vMat.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                   sizeof(pushConstants), &pushConstants);

                vkCmdDrawIndexed(frame.commandBuffer, mesh.mesh->indices.size(), 1, 0, 0, 0);
            }

            if (actor->hasCollision()) {
                Mesh* mesh = actor->actorCollider->getRenderMesh();
                VMaterial vMat = materialList.get(collisionMat.materialId);

                if (prevVMat != collisionMat.materialId) {
                    prevVMat = collisionMat.materialId;

                    vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipeline);
                    vkCmdBindDescriptorSets(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipelineLayout,
                                            0, 1, &frame.globalDescriptor, 0, nullptr);
                }

                AllocatedBuffer vertBuffer = this->bufferList.get(mesh->verticesId);
                AllocatedBuffer indBuffer = this->bufferList.get(mesh->indicesId);

                VkDeviceSize offset = 0;
                vkCmdBindVertexBuffers(frame.commandBuffer, 0, 1, &vertBuffer.buffer, &offset);
                vkCmdBindIndexBuffer(frame.commandBuffer, indBuffer.buffer, offset, VkIndexType::VK_INDEX_TYPE_UINT32);

                glm::mat4 model = glm::translate(actor->actorCollider->getRenderMeshTransform(), actor->getPosition());

                MeshPushConstants pushConstants = {
                        model,
                        actor->actorCollider->isColliding
                            ? glm::vec4(0.f, 1.f, 0.f, 1.f)
                            : glm::vec4(1.f, 0.f, 0.f, 1.f)
                };
                vkCmdPushConstants(frame.commandBuffer, vMat.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                   sizeof(pushConstants), &pushConstants);

                vkCmdDrawIndexed(frame.commandBuffer, mesh->indices.size(), 1, 0, 0, 0);
            }
        }

        vkCmdEndRenderPass(frame.commandBuffer);
    }
    vkEndCommandBuffer(frame.commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submitInfo.pWaitDstStageMask = &waitStage;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &frame.presentSemaphore;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &frame.renderSemaphore;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.commandBuffer;

    vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, frame.renderFence);

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

AllocatedBuffer VulkanRenderer::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;

    bufferCreateInfo.size = allocSize;
    bufferCreateInfo.usage = usage;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = memoryUsage;

    AllocatedBuffer allocatedBuffer;

    if (vmaCreateBuffer(this->allocator, &bufferCreateInfo, &allocationCreateInfo, &allocatedBuffer.buffer, &allocatedBuffer.allocation, nullptr) != VK_SUCCESS) {
        Logger::warn("Failed to allocate buffer");
        return {VK_NULL_HANDLE, VK_NULL_HANDLE};
    }

    return allocatedBuffer;
}

void VulkanRenderer::uploadMesh(Mesh& mesh) {
    AllocatedBuffer allocatedBuffer = this->createBuffer(mesh.vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    mesh.verticesId = this->bufferList.insert(allocatedBuffer);

    void* data;
    vmaMapMemory(this->allocator, allocatedBuffer.allocation, &data);
    memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
    vmaUnmapMemory(this->allocator, allocatedBuffer.allocation);

    allocatedBuffer = this->createBuffer(mesh.indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    mesh.indicesId = this->bufferList.insert(allocatedBuffer);
    vmaMapMemory(this->allocator, allocatedBuffer.allocation, &data);
    memcpy(data, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));
    vmaUnmapMemory(this->allocator, allocatedBuffer.allocation);
}

bool VulkanRenderer::createMaterial(Material& material) {
    VkShaderModule frag;
    if (!this->loadShader(FileUtils::getAssetsPath() + material.spirvFrag, &frag)) {
        Logger::error("Failed to open frag shader: " + material.spirvFrag);
        return false;
    }

    VkShaderModule vert;
    if (!this->loadShader(FileUtils::getAssetsPath() + material.spirvVert, &vert)) {
        Logger::error("Failed to open vert shader: " + material.spirvVert);
        return false;
    }

    VkPipelineLayout pipelineLayout;
    {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.pNext = nullptr;

        pipelineLayoutCreateInfo.flags = 0;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &this->globalDescriptorSetLayout;

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

    std::optional<VkPipeline> pipeline = PipelineBuilder(this->device, this->renderPass, pipelineLayout)
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vert)
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, frag)
            .setVertexInputInfo(VertexDescription::getVertexDescription())
            .setInputAssemblyInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .setViewport(0.f, 0.f, (float) this->settings->windowWidth, (float) this->settings->windowHeight)
            .setScissor(0, 0, this->settings->windowWidth, this->settings->windowHeight)
            .setRasterisationState(material.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL)
            .setMultisampleStateDefault()
            .setColourBlendAttachmentDefault()
            .setDepthStencilState(true, true, VK_COMPARE_OP_LESS, false)
            .buildPipeline();

    if (!pipeline.has_value()) {
        Logger::error("Failed to create pipeline");
        return false;
    }

    material.materialId = this->materialList.insert(VMaterial(pipeline.value(), pipelineLayout));

    vkDestroyShaderModule(this->device, frag, nullptr);
    vkDestroyShaderModule(this->device, vert, nullptr);

    return true;
}

FrameData& VulkanRenderer::getCurrentFrame() {
    return frameData[currentFrame % settings->bufferCount];
}

bool VulkanRenderer::registerMesh(Mesh* mesh) {
    if (mesh->verticesId && mesh->indicesId) return false;

    uploadMesh(*mesh);

    return true;
}

bool VulkanRenderer::registerMaterial(Material* material) {
    if (material->materialId) return false;

    createMaterial(*material);

    return true;
}
