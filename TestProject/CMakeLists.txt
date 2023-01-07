cmake_minimum_required(VERSION 3.22)

add_executable(leicester-game main.cpp)

target_link_libraries(leicester-game leicester-engine)

target_sources(leicester-game PRIVATE
        ControlledActor.cpp
)

set(assetDest "${CMAKE_CURRENT_BINARY_DIR}/Assets"  CACHE INTERNAL "")