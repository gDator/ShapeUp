cmake_minimum_required(VERSION 3.28)
project(ShapeUp)
include_directories(./include)
set(CMAKE_CXX_STANDARD 20)
set(SHAPE_UP_SOURCE
        include/dark.h
        src/main.cpp
        include/ShaderLoader.h
        src/ShaderLoader.cpp
        include/imconfig.h
        src/SDFCreatorGui.cpp
        include/SDFCreatorGui.h
        include/SDFCreator.h
        src/SDFCreator.cpp
        include/SDFObject.h
        include/SDFTypes.h
        include/SDFUtil.h
)
add_subdirectory(./vendor)
add_executable(ShapeUp
        ${SHAPE_UP_SOURCE})

target_include_directories(ShapeUp PUBLIC ./include)
target_link_libraries(ShapeUp raylib raylibImGui imgui stlplus_containers)
