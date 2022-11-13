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

bool VulkanRenderer::initialise(EngineSettings& settings) {
    Renderer::initialise(settings);

    if (!this->initVulkan(settings)) return false;
    if (!this->initSwapchain(settings)) return false;
    if (!this->initRenderpass(settings)) return false;
    if (!this->initFramebuffers(settings)) return false;
    if (!this->initSyncObjects(settings)) return false;
    this->initRender(settings);

    return true;
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
        auto deviceSelector = vkb::PhysicalDeviceSelector(vkbInstance)
                .set_minimum_version(1, 1)
                .set_surface(surface)
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

    // Command buffers
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.pNext = nullptr;

        commandPoolCreateInfo.queueFamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(this->device, &commandPoolCreateInfo, nullptr, &this->graphicsCommandPool) != VK_SUCCESS) {
            Logger::error("Failed to create command pool");
            return false;
        }
        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.pNext = nullptr;

        commandBufferAllocateInfo.commandPool = this->graphicsCommandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if (vkAllocateCommandBuffers(this->device, &commandBufferAllocateInfo, &this->commandBuffer) != VK_SUCCESS) {
            Logger::error("Failed to create command buffer");
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

    this->swapchain = vkbSwapchain.swapchain;
    this->swapchainImages = vkbSwapchain.get_images().value();
    this->swapchainImageViews = vkbSwapchain.get_image_views().value();

    this->swapchainImageFormat = vkbSwapchain.image_format;

    // Depth Buffer
    {
        VkExtent3D depthExtent = {settings.windowWidth, settings.windowHeight, 1};
        this->depthFormat = VK_FORMAT_D32_SFLOAT;
        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.pNext = nullptr;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = this->depthFormat;
        imageCreateInfo.extent = depthExtent;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocationCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vmaCreateImage(this->allocator, &imageCreateInfo, &allocationCreateInfo, &this->depthImage.image, &this->depthImage.allocation, nullptr);

        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = nullptr;

        imageViewCreateInfo.image = this->depthImage.image;
        imageViewCreateInfo.format = this->depthFormat;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (vkCreateImageView(this->device, &imageViewCreateInfo, nullptr, &this->depthImageView) != VK_SUCCESS) {
            Logger::error("Failed to create depth imageview");
            return false;
        }
    }
    return true;
}

bool VulkanRenderer::initRenderpass(EngineSettings& settings) {
    VkAttachmentDescription colourAttachment = {};
    {
        colourAttachment.format = swapchainImageFormat;
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

bool VulkanRenderer::initFramebuffers(EngineSettings& settings) {
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;

    framebufferCreateInfo.renderPass = this->renderPass;
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.width = settings.windowWidth;
    framebufferCreateInfo.height = settings.windowHeight;
    framebufferCreateInfo.layers = 1;

    const uint32_t swapchainLength = this->swapchainImages.size();
    this->framebuffers = std::vector<VkFramebuffer>(swapchainLength);

    for (int i = 0; i < swapchainLength; ++i) {
        VkImageView attachments[2] = {this->swapchainImageViews[i], depthImageView};
        framebufferCreateInfo.pAttachments = attachments;
        if (vkCreateFramebuffer(this->device, &framebufferCreateInfo, nullptr, &this->framebuffers[i]) != VK_SUCCESS) {
            Logger::error("Failed to create framebuffer");
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::initSyncObjects(EngineSettings& settings) {
    // Fence
    {
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(this->device, &fenceCreateInfo, nullptr, &renderFence) != VK_SUCCESS) {
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

        if (vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr, &renderSemaphore) != VK_SUCCESS
            || vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr, &presentSemaphore) != VK_SUCCESS) {
            Logger::error("Failed to create semaphore");
            return false;
        }
    }
    return true;
}



bool VulkanRenderer::initRender(EngineSettings& settings) {
    StaticMesh monkey;
    monkey.mesh = new Mesh();
    monkey.mesh->loadMeshFromFile(FileUtils::getAssetsPath() + "/Monkey.lmesh");
    monkey.material = new Material("/meshtriangle.vert.spv", "/colourtriangle.frag.spv");

    this->uploadMesh(*monkey.mesh);
    this->createMaterial(*monkey.material);

    this->renderables.push_back(monkey);

    StaticMesh triangle;
    triangle.mesh = new Mesh;
    triangle.mesh->vertices.resize(3);
    triangle.mesh->vertices[0].position = {0.5f, 0.5f, 0.f};
    triangle.mesh->vertices[1].position = {-0.5f, 0.5f, 0.f};
    triangle.mesh->vertices[2].position = {0.f, -0.5f, 0.f};

    triangle.mesh->vertices[0].colour = {1.f, 0.f, 0.f};
    triangle.mesh->vertices[1].colour = {0.f, 1.f, 0.f};
    triangle.mesh->vertices[2].colour = {0.f, 0.f, 1.f};

    triangle.mesh->indices = {0, 1, 2};

    triangle.material = new Material("/meshtriangle.vert.spv", "/colourtriangle.frag.spv");
    this->uploadMesh(*triangle.mesh);
    this->createMaterial(*triangle.material);

    this->renderables.push_back(triangle);

    return true;
}

void VulkanRenderer::cleanupSwapchain() {
    vkDeviceWaitIdle(this->device);

    vkDestroyImageView(this->device, this->depthImageView, nullptr);
    vmaDestroyImage(this->allocator, this->depthImage.image, this->depthImage.allocation);

    vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);

    for (int i = 0; i < this->swapchainImageViews.size(); ++i) {
        vkDestroyFramebuffer(this->device, this->framebuffers[i], nullptr);

        vkDestroyImageView(this->device, this->swapchainImageViews[i], nullptr);
    }
}

void VulkanRenderer::cleanup() {
    vkDeviceWaitIdle(this->device);

    cleanupSwapchain();

    for (auto& allocatedBuffer: bufferList.getMap()) {
        vmaDestroyBuffer(this->allocator, allocatedBuffer.second.buffer, allocatedBuffer.second.allocation);
    }
    bufferList.clear();

    for (auto& allocatedBuffer: materialList.getMap()) {
        allocatedBuffer.second.deleteMaterial(device);
    }
    bufferList.clear();

    deletionQueue.flush(this->device);

    vkDestroyFence(this->device, this->renderFence, nullptr);
    vkDestroySemaphore(this->device, this->renderSemaphore, nullptr);
    vkDestroySemaphore(this->device, this->presentSemaphore, nullptr);

    vkDestroyRenderPass(this->device, this->renderPass, nullptr);

    vkDestroyCommandPool(this->device, this->graphicsCommandPool, nullptr);

    vmaDestroyAllocator(this->allocator);

    vkDestroyDevice(this->device, nullptr);
    vkDestroySurfaceKHR(this->vInstance, this->surface, nullptr);

    vkb::destroy_debug_utils_messenger(this->vInstance, this->debugMessenger);
    vkDestroyInstance(this->vInstance, nullptr);

    Renderer::cleanup();
}

void VulkanRenderer::drawFrame(const double deltaTime, const double gameTime) {
    vkWaitForFences(this->device, 1, &renderFence, true, UINT64_MAX);
    vkResetFences(this->device, 1, &renderFence);

    uint32_t swapchainIndex;
    vkAcquireNextImageKHR(this->device, this->swapchain, UINT64_MAX, this->presentSemaphore, VK_NULL_HANDLE, &swapchainIndex);

    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(this->commandBuffer, &commandBufferBeginInfo);

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
    renderPassBeginInfo.framebuffer = this->framebuffers[swapchainIndex];

    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    // Renderpass
    {
        vkCmdBeginRenderPass(this->commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        glm::vec3 camPos = {0.f, 0.f, -2};
        glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
        glm::mat4 proj = glm::perspective(glm::radians(90.f), 1366.f/768.f, 0.1f, 200.f);
        int multiplier = 1;

        for (auto& mesh : renderables) {
            VMaterial vMat = materialList.get(mesh.material->materialId);
            vkCmdBindPipeline(this->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vMat.pipeline);

            AllocatedBuffer vertBuffer = this->bufferList.get(mesh.mesh->verticesIndex);
            AllocatedBuffer indBuffer = this->bufferList.get(mesh.mesh->indicesIndex);

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(this->commandBuffer, 0, 1, &vertBuffer.buffer, &offset);
            vkCmdBindIndexBuffer(this->commandBuffer, indBuffer.buffer, offset, VkIndexType::VK_INDEX_TYPE_UINT32);

            glm::mat4 model = glm::rotate(glm::mat4{1.f}, glm::radians((float) gameTime * 100 * multiplier), glm::vec3(0, 1, 0));
            multiplier *= -1;
            glm::mat4 renderMatrix = proj * view * model;

            MeshPushConstants pushConstants = {renderMatrix};
            vkCmdPushConstants(this->commandBuffer, vMat.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstants), &pushConstants);

            vkCmdDrawIndexed(this->commandBuffer, mesh.mesh->indices.size(), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(this->commandBuffer);
    }
    vkEndCommandBuffer(this->commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submitInfo.pWaitDstStageMask = &waitStage;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &this->presentSemaphore;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &this->renderSemaphore;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &this->commandBuffer;

    vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, this->renderFence);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;

    presentInfo.pSwapchains = &this->swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &this->renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapchainIndex;

    vkQueuePresentKHR(this->graphicsQueue, &presentInfo);
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

bool VulkanRenderer::uploadMesh(Mesh& mesh) {
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = mesh.vertices.size() * sizeof(Vertex);
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    AllocatedBuffer allocatedBuffer;

    if (vmaCreateBuffer(this->allocator, &bufferCreateInfo, &allocationCreateInfo, &allocatedBuffer.buffer, &allocatedBuffer.allocation, nullptr) != VK_SUCCESS) {
        Logger::warn("Failed to create upload mesh vertices to GPU");
        return false;
    }

    mesh.verticesIndex = this->bufferList.insert(allocatedBuffer);

    void* data;
    vmaMapMemory(this->allocator, allocatedBuffer.allocation, &data);
    memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
    vmaUnmapMemory(this->allocator, allocatedBuffer.allocation);

    bufferCreateInfo.size = mesh.indices.size() * sizeof(uint32_t);
    bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    allocatedBuffer = AllocatedBuffer();

    if (vmaCreateBuffer(this->allocator, &bufferCreateInfo, &allocationCreateInfo, &allocatedBuffer.buffer, &allocatedBuffer.allocation, nullptr) != VK_SUCCESS) {
        Logger::warn("Failed to create upload mesh indices to GPU");
        return false;
    }

    mesh.indicesIndex = this->bufferList.insert(allocatedBuffer);
    vmaMapMemory(this->allocator, allocatedBuffer.allocation, &data);
    memcpy(data, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));
    vmaUnmapMemory(this->allocator, allocatedBuffer.allocation);

    return true;
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
        pipelineLayoutCreateInfo.setLayoutCount = 0;
        pipelineLayoutCreateInfo.pSetLayouts = nullptr;

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
            .setRasterisationState(VK_POLYGON_MODE_FILL)
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

