cmake_minimum_required(VERSION 3.22)

project(leicester-engine VERSION 1.0.0 DESCRIPTION "Top Level")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 1)

# Ensure debug flag is set when posix
IF(NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
ENDIF()

add_library(leicester-engine STATIC)

add_subdirectory(Third-Party)
add_subdirectory(AssetProcessor)
add_subdirectory(Engine)

set(projectAssetDir "" CACHE FILEPATH "The asset directory for the project")
set(assetDest "${CMAKE_BINARY_DIR}/Engine" CACHE FILEPATH "Destination Path of the asset directory in the compiled project")

# Testing
add_subdirectory(TestProject)