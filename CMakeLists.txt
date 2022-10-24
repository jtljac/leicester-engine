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

add_subdirectory(Engine)

set(assetDest "${CMAKE_BINARY_DIR}/Engine" CACHE FILEPATH "Path to the asset directory")

# Testing
add_subdirectory(TestProject)