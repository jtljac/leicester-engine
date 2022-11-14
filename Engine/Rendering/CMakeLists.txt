cmake_minimum_required(VERSION 3.22)

target_sources(leicester-engine PRIVATE ./Source/Renderer.cpp)

add_subdirectory(GPUStructures)
add_subdirectory(Vulkan)