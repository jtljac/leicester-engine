cmake_minimum_required(VERSION 3.22)

# GLM for Maths
find_package(glm REQUIRED)
target_link_libraries(leicester-engine glm)

# Vulkan for vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(leicester-engine vulkan)

# vk-bootstrap to setup vulkan
add_subdirectory(vk-bootstrap)
target_link_libraries(leicester-engine vk-bootstrap::vk-bootstrap)

add_subdirectory(vulkan-memory-manager)
target_link_libraries(leicester-engine VulkanMemoryAllocator)

# GLFW for window and Inputs
find_package(glfw3 3.3 REQUIRED)
target_link_libraries(leicester-engine glfw)