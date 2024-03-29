/**
 * This code was created by following the following guide:
 * https://vkguide.dev/docs/chapter-3/scene_management/  MIT Licence: https://github.com/vblanco20-1/vulkan-guide/blob/master/LICENSE.txt
 */
#pragma once

#include <vulkan/vulkan.h>
#include <Material/Material.h>

struct VMaterial {
    VkDescriptorSetLayout materialDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet materialDescriptor = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    VMaterial(VkPipeline pipeline, VkPipelineLayout pipelineLayout,
              VkDescriptorSetLayout materialDescriptorSetLayout, VkDescriptorSet materialDescriptorSet);

    void deleteMaterial(VkDevice device);
};

namespace MaterialUtil {
    VkShaderStageFlagBits stageFlagFromShaderStage(ShaderStage shaderStage);
}