cmake_minimum_required(VERSION 3.22)

add_subdirectory(Utils)
add_subdirectory(Engine)
add_subdirectory(Platform)
add_subdirectory(Material)
add_subdirectory(Mesh)
add_subdirectory(Collision)
add_subdirectory(Scene)
add_subdirectory(Rendering)

target_include_directories(leicester-engine PUBLIC ./)

target_sources(leicester-engine PRIVATE "Source/LeicesterEngine.cpp")

add_custom_target(assetProcessor
        COMMAND asset-processor "${CMAKE_CURRENT_LIST_DIR}/Assets" "${assetDest}"
        COMMENT "Running Asset Processor"
        )
add_dependencies(leicester-engine assetProcessor)