cmake_minimum_required(VERSION 3.22)

project(asset-processor)

add_executable(asset-processor main.cpp)

add_subdirectory(Third-Party)
add_subdirectory(FileProcessors)