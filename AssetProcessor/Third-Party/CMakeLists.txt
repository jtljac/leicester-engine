cmake_minimum_required(VERSION 3.22)

include(FetchContent)

# tinyobjloader for reading .obj files
target_include_directories(asset-processor PUBLIC tinyobjloader)

# GLM for Maths
find_package(glm REQUIRED)
target_link_libraries(asset-processor glm)


## Shaderc for compiling shaders
#set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
#set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
#set(SHADERC_GLSLANG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glslang" CACHE PATH "" FORCE)
#add_subdirectory(shaderc)
#target_link_libraries(asset-processor shaderc_combined)