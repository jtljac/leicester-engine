cmake_minimum_required(VERSION 3.22)

# tinyobjloader for reading .obj files
target_include_directories(asset-processor PUBLIC tinyobjloader)

# stb_image for reading image files
target_include_directories(asset-processor PUBLIC stb)

# GLM for Maths
find_package(glm REQUIRED)
target_link_libraries(asset-processor glm)

# glslang for ShaderC
add_subdirectory(glslang)

# SPIRV Tools for shaderc
set(SPIRV_HEADERS_SKIP_INSTALL OFF)
set(SPIRV_HEADERS_SKIP_EXAMPLES ON)
add_subdirectory(SPIRV-Headers)
add_subdirectory(SPIRV-Tools)

# Shaderc for compiling shaders
set(SHADERC_SKIP_INSTALL ON)
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_EXAMPLES ON)
set(SHADERC_ENABLE_COPYRIGHT_CHECK OFF)
add_subdirectory(shaderc)

target_link_libraries(asset-processor shaderc)