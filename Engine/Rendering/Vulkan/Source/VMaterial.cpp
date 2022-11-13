//
// Created by jacob on 13/11/22.
//

#include "../VMaterial.h"

VMaterial::VMaterial(VkPipeline pipeline, VkPipelineLayout pipelineLayout) : pipeline(pipeline), pipelineLayout(pipelineLayout) {}

void VMaterial::deleteMaterial(VkDevice device) {
    if (pipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, this->pipeline, nullptr);
    if (pipelineLayout != VK_NULL_HANDLE) vkDestroyPipelineLayout(device, this->pipelineLayout, nullptr);
}