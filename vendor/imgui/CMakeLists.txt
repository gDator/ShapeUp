add_library(imgui
        src/imgui.cpp
        src/imgui_draw.cpp
        src/imgui_stdlib.cpp
        src/imgui_tables.cpp
        src/imgui_widgets.cpp
        src/implot.cpp
        src/implot_items.cpp
        src/imgui_demo.cpp
        src/imgui_spectrum.cpp)
target_include_directories(imgui PUBLIC ./include)