#include <cmath>
#include <fstream>

#include "../VulkanRenderer.h"
#include "../PipelineBuilder.h"
#include <Utils/Logger.h>
#include <Utils/FileUtils.h>

#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

bool VulkanRenderer::initialise(EngineSettings& settings) {
    Renderer::initialise(settings);

    if (!this->initVulkan(settings)) return false;
    if (!this->initSwapchain(settings)) return false;
    if (!this->initRenderpass(settings)) return false;
    if (!this->initFramebuffers(settings)) return false;
    if (!this->initSyncObjects(settings)) return false;
    if (!this->initPipelines(settings)) return false;

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

    return true;
}

bool VulkanRenderer::initRenderpass(EngineSettings& settings) {
    VkAttachmentDescription colourAttachment = {};
    colourAttachment.format = swapchainImageFormat;
    colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colourAttachmentReference = {};

    colourAttachmentReference.attachment = 0;
    colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colourAttachmentReference;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colourAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;

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
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.width = settings.windowWidth;
    framebufferCreateInfo.height = settings.windowHeight;
    framebufferCreateInfo.layers = 1;

    const uint32_t swapchainLength = this->swapchainImages.size();
    this->framebuffers = std::vector<VkFramebuffer>(swapchainLength);

    for (int i = 0; i < swapchainLength; ++i) {
        framebufferCreateInfo.pAttachments = &this->swapchainImageViews[i];
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



bool VulkanRenderer::initPipelines(EngineSettings& settings) {
    VkShaderModule triangleFrag;
    if (!this->loadShader(FileUtils::getAssetsPath() + "/triangle.frag.spv", &triangleFrag)) {
        Logger::error("Failed to open triangle frag shader");
        return false;
    }

    VkShaderModule triangleVert;
    if (!this->loadShader(FileUtils::getAssetsPath() + "/triangle.vert.spv", &triangleVert)) {
        Logger::error("Failed to open triangle vert shader");
        return false;
    }

    VkShaderModule colourTriangleFrag;
    if (!this->loadShader(FileUtils::getAssetsPath() + "/colourtriangle.frag.spv", &colourTriangleFrag)) {
        Logger::error("Failed to open triangle frag shader");
        return false;
    }

    VkShaderModule colourTriangleVert;
    if (!this->loadShader(FileUtils::getAssetsPath() + "/colourtriangle.vert.spv", &colourTriangleVert)) {
        Logger::error("Failed to open triangle vert shader");
        return false;
    }

    {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.pNext = nullptr;

        pipelineLayoutCreateInfo.flags = 0;
        pipelineLayoutCreateInfo.setLayoutCount = 0;
        pipelineLayoutCreateInfo.pSetLayouts = nullptr;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(this->device, &pipelineLayoutCreateInfo, nullptr, &this->trianglePipelineLayout) !=
            VK_SUCCESS) {
            Logger::error("Failed to create pipeline layout");
            return false;
        }
    }

    PipelineBuilder pipelineBuilder = PipelineBuilder(this->device, this->renderPass, this->trianglePipelineLayout)
        .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, triangleVert)
        .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFrag)
        .setVertexInputInfoDefault()
        .setInputAssemblyInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .setViewport(0.f, 0.f, (float) settings.windowWidth, (float) settings.windowHeight)
        .setScissor(0, 0, settings.windowWidth, settings.windowHeight)
        .setRasterisationState(VK_POLYGON_MODE_FILL)
        .setMultisampleStateDefault()
        .setColourBlendAttachmentDefault();

    std::optional<VkPipeline> pipeline = pipelineBuilder.buildPipeline();

    if (!pipeline.has_value()) {
        Logger::error("Failed to create pipeline");
        return false;
    }

    this->trianglePipeline = pipeline.value();

    std::optional<VkPipeline> colourPipeline = pipelineBuilder
        .clearShaderStages()
        .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, colourTriangleVert)
        .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, colourTriangleFrag)
        .buildPipeline();

    if (!colourPipeline.has_value()) {
        Logger::error("Failed to create pipeline");
        return false;
    }

    this->colourTrianglePipeline = colourPipeline.value();

    vkDestroyShaderModule(this->device, triangleFrag, nullptr);
    vkDestroyShaderModule(this->device, triangleVert, nullptr);
    vkDestroyShaderModule(this->device, colourTriangleFrag, nullptr);
    vkDestroyShaderModule(this->device, colourTriangleVert, nullptr);

    this->deletionQueue.pushDeletor([trianglePipeline = this->trianglePipeline, colourTrianglePipeline = this->colourTrianglePipeline](VkDevice& device) {
        vkDestroyPipeline(device, trianglePipeline, nullptr);
        vkDestroyPipeline(device, colourTrianglePipeline, nullptr);
    });

    return true;
}

void VulkanRenderer::cleanupSwapchain() {
    vkDeviceWaitIdle(this->device);
    vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);

    for (int i = 0; i < this->swapchainImageViews.size(); ++i) {
        vkDestroyFramebuffer(this->device, this->framebuffers[i], nullptr);

        vkDestroyImageView(this->device, this->swapchainImageViews[i], nullptr);
    }
}

void VulkanRenderer::cleanup() {
    vkDeviceWaitIdle(this->device);

    cleanupSwapchain();

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

    VkClearValue clearValue;
    float flash = (float) std::abs(std::sin(gameTime));
    clearValue.color = {{0.f, 0.f, flash, 1.f}};

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;

    renderPassBeginInfo.renderPass = this->renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = {1366, 768};
    renderPassBeginInfo.framebuffer = this->framebuffers[swapchainIndex];

    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(this->commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    if (this->shader == 0) {
        vkCmdBindPipeline(this->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->trianglePipeline);
    } else {
        vkCmdBindPipeline(this->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->colourTrianglePipeline);
    }
    vkCmdDraw(this->commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(this->commandBuffer);
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
