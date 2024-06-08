add_library(FileDialog
        FileDialog.cpp
        FileDialog.h)
target_include_directories(FileDialog PUBLIC ./)
target_link_libraries(FileDialog PUBLIC raylib)