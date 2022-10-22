//
// Created by jacob on 21/10/22.
//

#include "../VulkanRenderer.h"
#include <Utils/Logger.h>

#include <VkBootstrap.h>

bool VulkanRenderer::initialise(EngineSettings& settings) {
    Renderer::initialise(settings);

    if (!this->initVulkan(settings)) return false;
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

void VulkanRenderer::drawFrame(double deltaTime) {

}

bool VulkanRenderer::initVulkan(EngineSettings& settings) {
    vkb::InstanceBuilder instanceBuilder;

    instanceBuilder
            .set_engine_name("Leicester Engine")
            .set_app_name(settings.windowTitle.c_str())
            .request_validation_layers(true)
            .require_api_version(1, 1, 0)
            .use_default_debug_messenger();

    auto systemInfo = vkb::SystemInfo::get_system_info();

    if (!systemInfo.has_value()) {
        Logger::error(systemInfo.error().message());
        return false;
    }
    for (const auto& layer: this->validationLayers){
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
        if (systemInfo->is_extension_available(glfwExtensions[i])) instanceBuilder.enable_layer(glfwExtensions[i]);
        else {
            // Need to construct the message in a silly way so we can concat the layer on the end
            std::string message = "Failed to find validation layer: ";
            message += glfwExtensions[i];
            Logger::warn(message);
        }
    }

    auto vkbInst = instanceBuilder.build();

    if (!vkbInst.has_value()) return false;

    vkb::Instance vkbInstance = vkbInst.value();

    this->vInstance = vkbInstance.instance;
    this->debugMessenger = vkbInstance.debug_messenger;

    // Surface
    VkResult result = glfwCreateWindowSurface(this->vInstance, this->window, nullptr, &surface);

    if (result != VK_SUCCESS) {
        Logger::error("Failed to create Surface");
        return false;
    }

    // Pick Device
    auto deviceSelector = vkb::PhysicalDeviceSelector(vkbInstance)
        .set_minimum_version(1, 1)
        .set_surface(surface)
        .add_required_extensions(this->deviceExtensions)
        .select();

    if (!deviceSelector.has_value()) return false;

    vkb::PhysicalDevice vkbPDevice = deviceSelector.value();

    auto vkbDev = vkb::DeviceBuilder(vkbPDevice)
            .build();

    vkb::Device vkbDevice = vkbDev.value();

    this->device = vkbDevice.device;
    this->gpu = vkbDevice.physical_device;
    return true;
}
