//
// Created by jacob on 13/11/22.
//

#pragma once

#include <vulkan/vulkan.h>
#include <Material/Material.h>

struct VMaterial {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;



    VMaterial(VkPipeline pipeline, VkPipelineLayout pipelineLayout);

    void deleteMaterial(VkDevice device);
};
