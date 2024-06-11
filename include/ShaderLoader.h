//
// Created by Daniel Hagen on 08.06.2024.
//

#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <filesystem>
class ShaderLoader
{
public:
    void loadShader();
    inline const std::string& getShaderPrefixFS() {return m_shader_prefix_fs;}
    inline const std::string& getShaderBaseFS() {return m_shader_base_fs;}
    inline const std::string& getSlicerBodyFS() {return m_slicer_body_fs;}
    inline const std::string& getSelectionFS() {return m_selection_fs;}

private:
    std::string m_shader_prefix_fs{};
    std::string m_shader_base_fs{};
    std::string m_slicer_body_fs{};
    std::string m_selection_fs{};
    static void loadFile(const std::filesystem::path& file_path, std::string& output);
};

#endif // SHADER_H
