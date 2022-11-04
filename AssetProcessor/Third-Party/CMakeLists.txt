cmake_minimum_required(VERSION 3.22)

# tinyobjloader
target_include_directories(asset-processor PUBLIC tinyobjloader)

# GLM for Maths
find_package(glm REQUIRED)
target_link_libraries(asset-processor glm)