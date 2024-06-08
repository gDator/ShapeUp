add_library(raylibImGui rlImGui.cpp)
target_include_directories(raylibImGui PUBLIC ./)
target_link_libraries(raylibImGui raylib imgui)