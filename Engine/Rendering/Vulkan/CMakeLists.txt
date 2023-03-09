cmake_minimum_required(VERSION 3.22)

target_sources(leicester-engine PRIVATE
        Source/VulkanRenderer.cpp
        Source/PipelineBuilder.cpp
        Source/DeletionQueue.cpp
        Source/VertexDescription.cpp
        Source/VMaterial.cpp
        Source/FrameStructures.cpp
        Source/VkShortcuts.cpp
        Source/VTexture.cpp
)