//
// Created by Daniel Hagen on 08.06.2024.
//
#include "shader.h"
#include <fstream>
#include <iostream>
void ShaderFiles::loadShader()
{
    loadFile("../src/shader_base.fs", m_shader_base_fs);
    loadFile("../src/shader_prefix.fs", m_shader_prefix_fs);
    loadFile("../src/slicer_body.fs", m_slicer_body_fs);
    loadFile("../src/selection.fs", m_selection_fs);
}

void ShaderFiles::loadFile(const std::filesystem::path& file_path, std::string& output)
{
    if(std::filesystem::exists(file_path))
    {
        std::ifstream file(file_path);

        while(!file.eof())
        {
            std::string buffer{};
            std::getline(file, buffer);
            output.append((buffer));
            output.append("\n");
        }
        file.close();
    }

    std::cout << output;
}