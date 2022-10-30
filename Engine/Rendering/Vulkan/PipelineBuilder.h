/**
 * This file (and it's sources) has been created by following this tutorial:
 * https://vkguide.dev/docs/chapter-2/pipeline_walkthrough/     MIT Licence: https://github.com/vblanco20-1/vulkan-guide/blob/master/LICENSE.txt
 *
 * The guide's example has been modified heavily to favour a better builder paradigm, specifically to loosely match VKBootstrap (https://github.com/charles-lunarg/vk-bootstrap)
 */
#pragma once

#include <vector>
#include <optional>

#include <vulkan/vulkan.h>
#include "VertexDescription.h"


/**
 * A builder for creating vulkan pipelines
 */
class PipelineBuilder {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    VkPipelineColorBlendAttachmentState colourBlendAttachment = {};
    VkPipelineMultisampleStateCreateInfo multisampleState = {};

    VkDevice device;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;

public:
    PipelineBuilder(VkDevice device, VkRenderPass renderPass, VkPipelineLayout pipelineLayout);

    /**
     * Add a shader stage to the list of stages
     * @param stage a bitmask representing the stage to run the shader at
     * @param shaderModule The compiled shader module being run at this stage
     * @param flags a bitmask that controls how the shader stage is created
     * @return the PipelineBuilder
     */
    PipelineBuilder& addShaderStage(VkShaderStageFlagBits stage, VkShaderModule shaderModule, VkPipelineShaderStageCreateFlags flags = 0);

    /**
     * Add a shader stage to the list of stages
     * @param shaderStageCreateInfo An already made VkPipelineShaderStageCreateInfo
     * @return the PipelineBuilder
     */
    PipelineBuilder& addShaderStage(VkPipelineShaderStageCreateInfo& shaderStageCreateInfo);

    /**
     * Remove all shader stages in the builder
     * @return the PipelineBuilder
     */
    PipelineBuilder& clearShaderStages();


    /**
     * Sets the vertexInputInfo such that vertexBindingDescriptionCount and vertexAttributeDescriptionCount are both set to 0
     * @return the PipelineBuilder
     */
    PipelineBuilder& setVertexInputInfoDefault();

    /**
     * Sets the vertexInputInfo using info from a VertexDescription
     * @param vertexDescription the vertexDescription to pull the vertex input info from
     * @return the PipelineBuilder
     */
    PipelineBuilder& setVertexInputInfo(VertexDescription vertexDescription);

    /**
     * Sets vertexInputInfo
     * @param vertexInputStateCreateInfo An already made VkPipelineVertexInputStateCreateInfo
     * @return the PipelineBuilder
     */
    PipelineBuilder& setVertexInputInfo(VkPipelineVertexInputStateCreateInfo& vertexInputStateCreateInfo);


    /**
     * Sets inputAssemblyInfo
     * @param topology The topology of the mesh
     * @return the PipelineBuilder
     */
    PipelineBuilder& setInputAssemblyInfo(VkPrimitiveTopology topology);

    /**
     * Sets inputAssemblyInfo
     * @param inputAssemblyStateCreateInfo An already made VkPipelineInputAssemblyStateCreateInfo
     * @return the PipelineBuilder
     */
    PipelineBuilder& setInputAssemblyInfo(VkPipelineInputAssemblyStateCreateInfo& inputAssemblyStateCreateInfo);


    /**
     * Set the viewport dimensions
     * @param x The x offset of the viewport
     * @param y The y offset of the viewport
     * @param width The width of the viewport
     * @param height The height of the viewport
     * @param minDepth The minimum depth of the viewport
     * @param maxDepth The maximum depth of the viewport
     * @return the PipelineBuilder
     */
    PipelineBuilder& setViewport(float x, float y, float width, float height, float minDepth = 0.f, float maxDepth = 1.f);
    /**
     * Set the viewport dimensions
     * @param viewport A ready made VkViewport
     * @return the PipelineBuilder
     */
    PipelineBuilder& setViewport(VkViewport viewport);


    /**
     * Set the scissor dimensions
     * @param x The x offset of the scissor
     * @param y The y offset of the scissor
     * @param width The width of the scissor
     * @param height The height of the sissor
     * @return the PipelineBuilder
     */
    PipelineBuilder& setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

    /**
     * Set the scissor dimensions
     * @param offset2D The offset of the scissor (x & y)
     * @param extent  The extent of the scissor (width & height)
     * @return the PipelineBuilder
     */
    PipelineBuilder& setScissor(VkOffset2D offset2D, VkExtent2D extent);

    /**
     * Set the scissor dimensions
     * @param scissor A ready made VkRect2dD
     * @return the PipelineBuilder
     */
    PipelineBuilder& setScissor(VkRect2D scissor);


    /**
     * Set the rasterization state
     * @param polygonMode The method inwhich the engine draws polygons
     * @return the PipelineBuilder
     */
    PipelineBuilder& setRasterisationState(VkPolygonMode polygonMode);

    /**
     * Set the rasterization state
     * @param rasterizationStateCreateInfo A ready made VkPipelineRasterzationStateCreateInfo
     * @return the PipelineBuilder
     */
    PipelineBuilder& setRasterisationState(VkPipelineRasterizationStateCreateInfo& rasterizationStateCreateInfo);


    /**
     * Set the multisampleState to no multisampling
     * @return the PipelineBuilder
     */
    PipelineBuilder& setMultisampleStateDefault();

    /**
     * Set the multisampleState
     * @param multisampleStateCreateInfo A ready made VkPipelineMultisampleStateCreateInfo
     * @return the PipelineBuilder
     */
    PipelineBuilder& setMultisampleState(VkPipelineMultisampleStateCreateInfo& multisampleStateCreateInfo);


    /**
     * Set the colourBlendAttachment to no blending
     * @return the PipelineBuilder
     */
    PipelineBuilder& setColourBlendAttachmentDefault();

    /**
     * Set the colorBlendAttachment
     * @param colourBlendAttachmentState A ready made VkPipelineColorBlendAttachmentState
     * @return the PipelineBuilder
     */
    PipelineBuilder& setColourBlendAttachment(VkPipelineColorBlendAttachmentState& colourBlendAttachmentState);

    /**
     * Build the pipeline
     * @return An optional containing the created pipeline
     */
    std::optional<VkPipeline> buildPipeline();
};
