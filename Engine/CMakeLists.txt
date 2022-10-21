cmake_minimum_required(VERSION 3.22)

add_subdirectory(Utils)
add_subdirectory(Platform)
add_subdirectory(Mesh)
add_subdirectory(Collision)
add_subdirectory(Scene)
add_subdirectory(Rendering)

target_include_directories(leicester-engine PUBLIC ./)

target_sources(leicester-engine PRIVATE "Source/LeicesterEngine.cpp")