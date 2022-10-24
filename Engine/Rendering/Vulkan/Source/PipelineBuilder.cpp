//
// Created by jacob on 24/10/22.
//

#include "Rendering/Vulkan/PipelineBuilder.h"

PipelineBuilder::PipelineBuilder(VkDevice device, VkRenderPass renderPass, VkPipelineLayout pipelineLayout) : device(device), renderPass(renderPass), pipelineLayout(pipelineLayout) {}

PipelineBuilder& PipelineBuilder::addShaderStage(VkShaderStageFlagBits stage, VkShaderModule shaderModule, VkPipelineShaderStageCreateFlags flags) {
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.pNext = nullptr;

    shaderStageCreateInfo.flags = flags;
    shaderStageCreateInfo.stage = stage;
    shaderStageCreateInfo.module = shaderModule;
    shaderStageCreateInfo.pName = "main";

    this->shaderStages.push_back(shaderStageCreateInfo);

    return *this;
}

PipelineBuilder& PipelineBuilder::addShaderStage(VkPipelineShaderStageCreateInfo& shaderStageCreateInfo) {
    this->shaderStages.push_back(shaderStageCreateInfo);

    return *this;
}

PipelineBuilder& PipelineBuilder::setVertexInputInfoDefault() {
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.pNext = nullptr;

    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;

    this->vertexInputInfo = vertexInputStateCreateInfo;

    return *this;
}

PipelineBuilder& PipelineBuilder::setInputAssemblyInfo(VkPrimitiveTopology topology) {
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.pNext = nullptr;

    inputAssemblyStateCreateInfo.topology = topology;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    this->inputAssemblyInfo = inputAssemblyStateCreateInfo;

    return *this;
}



PipelineBuilder& PipelineBuilder::setInputAssemblyInfo(VkPipelineInputAssemblyStateCreateInfo& inputAssemblyStateCreateInfo) {
    this->inputAssemblyInfo = inputAssemblyStateCreateInfo;

    return *this;
}

PipelineBuilder& PipelineBuilder::setViewport(float x, float y, float width, float height, float minDepth, float maxDepth) {
    this->viewport = {x, y, width, height, minDepth, maxDepth};

    return *this;
}

PipelineBuilder& PipelineBuilder::setViewport(VkViewport viewport) {
    this->viewport = viewport;

    return *this;
}

PipelineBuilder& PipelineBuilder::setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) {
    this->scissor = {{x, y}, {width, height}};

    return *this;}

PipelineBuilder& PipelineBuilder::setScissor(VkOffset2D offset2D, VkExtent2D extent) {
    this->scissor = {offset2D, extent};

    return *this;
}

PipelineBuilder& PipelineBuilder::setScissor(VkRect2D scissor) {
    this->scissor = scissor;

    return *this;
}

PipelineBuilder& PipelineBuilder::setRasterisationState(VkPolygonMode polygonMode) {
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.pNext = nullptr;

    rasterizationStateCreateInfo.polygonMode = polygonMode;

    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.lineWidth = 1.f;

    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.f;
    rasterizationStateCreateInfo.depthBiasClamp = 0.f;
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.f;

    this->rasterizationState = rasterizationStateCreateInfo;

    return *this;
}

PipelineBuilder&
PipelineBuilder::setRasterisationState(VkPipelineRasterizationStateCreateInfo& rasterizationStateCreateInfo) {
    this->rasterizationState = rasterizationStateCreateInfo;

    return *this;
}

PipelineBuilder& PipelineBuilder::setMultisampleStateDefault() {
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.pNext = nullptr;

    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateCreateInfo.minSampleShading = 1.f;
    multisampleStateCreateInfo.pSampleMask = nullptr;
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

    this->multisampleState = multisampleStateCreateInfo;

    return *this;
}

PipelineBuilder& PipelineBuilder::setMultisampleState(VkPipelineMultisampleStateCreateInfo& multisampleStateCreateInfo) {
    this->multisampleState = multisampleStateCreateInfo;

    return *this;
}

PipelineBuilder& PipelineBuilder::setColourBlendAttachmentDefault() {
    VkPipelineColorBlendAttachmentState colourBlendAttachmentState = {};
    colourBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_A_BIT;

    colourBlendAttachmentState.blendEnable = VK_FALSE;

    this->colourBlendAttachment = colourBlendAttachmentState;

    return *this;
}

PipelineBuilder& PipelineBuilder::setColourBlendAttachment(VkPipelineColorBlendAttachmentState& colourBlendAttachmentState) {
    this->colourBlendAttachment = colourBlendAttachmentState;

    return *this;
}

std::optional<VkPipeline> PipelineBuilder::buildPipeline() {
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    {
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.pNext = nullptr;

        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.pViewports = &this->viewport;
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = &this->scissor;
    }

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
    {
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.pNext = nullptr;

        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &this->colourBlendAttachment;
    }


    VkPipeline pipeline;
    {
        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.pNext = nullptr;
        pipelineCreateInfo.stageCount = this->shaderStages.size();
        pipelineCreateInfo.pStages = this->shaderStages.data();

        pipelineCreateInfo.pVertexInputState = &this->vertexInputInfo;
        pipelineCreateInfo.pInputAssemblyState = &this->inputAssemblyInfo;
        pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        pipelineCreateInfo.pRasterizationState = &this->rasterizationState;
        pipelineCreateInfo.pMultisampleState = &this->multisampleState;
        pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
        pipelineCreateInfo.layout = this->pipelineLayout;
        pipelineCreateInfo.renderPass = this->renderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
            return std::nullopt;
        }
    }

    return {pipeline};
}
