//
// Created by jacob on 13/11/22.
//

#include "../VMaterial.h"

VMaterial::VMaterial(VkPipeline pipeline, VkPipelineLayout pipelineLayout,
                     VkDescriptorSetLayout materialDescriptorSetLayout, VkDescriptorSet materialDescriptorSet)
        : pipeline(pipeline), pipelineLayout(pipelineLayout), materialDescriptorSetLayout(materialDescriptorSetLayout),
          materialDescriptor(materialDescriptorSet) {}

void VMaterial::deleteMaterial(VkDevice device) {
    if (pipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, this->pipeline, nullptr);
    if (pipelineLayout != VK_NULL_HANDLE) vkDestroyPipelineLayout(device, this->pipelineLayout, nullptr);
    if (materialDescriptorSetLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(device, materialDescriptorSetLayout, nullptr);
}

VkShaderStageFlagBits MaterialUtil::stageFlagFromShaderStage(ShaderStage shaderStage) {
    switch (shaderStage) {
        case ShaderStage::FRAG:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::VERT:
            return VK_SHADER_STAGE_VERTEX_BIT;
    }
}
