//
// Created by Daniel Hagen on 08.06.2024.
//

#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <filesystem>
class ShaderFiles
{
public:
    void loadShader();
    inline const char* getShaderPrefixFS() {return m_shader_prefix_fs.c_str();}
    inline const char* getShaderBaseFS() {return m_shader_base_fs.c_str();}
    inline const char* getSlicerBodyFS() {return m_slicer_body_fs.c_str();}
    inline const char* getSelectionFS() {return m_selection_fs.c_str();}

private:
    std::string m_shader_prefix_fs{};
    std::string m_shader_base_fs{};
    std::string m_slicer_body_fs{};
    std::string m_selection_fs{};
    static void loadFile(const std::filesystem::path& file_path, std::string& output);
};

#endif // SHADER_H
