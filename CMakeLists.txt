cmake_minimum_required(VERSION 3.28)
project(ShapeUp)
include_directories(./src)
set(CMAKE_CXX_STANDARD 20)
set(SHAPE_UP_SOURCE
        src/dark.h
        src/main.cpp
        src/shader.h
        src/shader.cpp
        src/imconfig.h
)
add_subdirectory(./vendor)
add_executable(ShapeUp
        ${SHAPE_UP_SOURCE})

target_include_directories(ShapeUp PUBLIC ./include)
target_link_libraries(ShapeUp raylib raylibImGui imgui raygui)
