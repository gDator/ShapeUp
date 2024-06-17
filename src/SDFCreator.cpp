//
// Created by Daniel Hagen on 10.06.2024.
//
#include "SDFCreator.h"

#include "SDFUtil.h"
#include "raylib.h"
#include "rlgl.h"
#include <cassert>
#include "raymath.h"

#include <fstream>
#include <iostream>
#include <external/glad.h>

SDFCreator::SDFCreator()
{
    m_last_save = GetTime();
    m_shader_files.loadShader();
    m_selected_sphere.set_null();
    const SDFObject root;
    m_sdf_tree.insert(root);
}
void SDFCreator::rebuildShaders()
{
    m_needs_rebuild = false;
    UnloadShader(m_main_shader);
    std::string map_function;
    if (m_selected_sphere.valid())
    {
        appendMapFunction(map_function, false, &(*m_selected_sphere));
    }
    else
    {
        appendMapFunction(map_function, false, nullptr);
    }

    std::string result;
    result += SHADER_VERSION_PREFIX "out vec4 finalColor;\n"
                                    "uniform vec3 viewEye; \n"
                                    "uniform vec3 viewCenter; \n"
                                    "uniform float runTime; \n"
                                    "uniform float visualizer; \n"
                                    "uniform vec2 resolution;\n"
                                    "uniform float blend;\n";

    result += m_shader_files.getShaderPrefixFS();
    result += map_function;
    result += m_shader_files.getShaderBaseFS();
    m_main_shader = LoadShaderFromMemory(vshader, result.c_str());

    m_main_locations.view_eye = GetShaderLocation(m_main_shader, "viewEye");
    m_main_locations.view_center = GetShaderLocation(m_main_shader, "viewCenter");
    m_main_locations.run_time = GetShaderLocation(m_main_shader, "runTime");
    m_main_locations.resolution = GetShaderLocation(m_main_shader, "resolution");
    m_main_locations.selected_params = GetShaderLocation(m_main_shader, "selectionValues");
    m_main_locations.visualizer = GetShaderLocation(m_main_shader, "visualizer");
    m_main_locations.blend = GetShaderLocation(m_main_shader, "blend");
    SaveFileText("./shader.glsl", result.data());
}

void SDFCreator::deleteObject()
{
    if (m_selected_sphere.valid())
    {
        // cut off subtree and append
        if (m_sdf_tree.children(m_selected_sphere) > 0)
        {
            const auto subtree = m_sdf_tree.subtree(m_selected_sphere, 0);
            const auto parent = m_sdf_tree.parent(m_selected_sphere);
            // add subtree
            if (parent.valid())
            {
                m_sdf_tree.insert(parent, subtree);
            }
            else
            {
                // add at root node
                m_sdf_tree.insert(subtree);
            }
        }
        m_sdf_tree.erase(m_selected_sphere);
        m_selected_sphere.set_null();
        m_needs_rebuild = true;
        m_num_spheres--;
    }
}

void SDFCreator::addShape(Color color)
{
    if (m_sdf_tree.size() <= 0)
    {
        SDFObject root;
        root.color = {color.r, color.g, color.b};
        m_sdf_tree.insert(root);
        return;
    }
    SDFObject obj;
    obj.size = {1, 1, 1};
    obj.color = {color.r, color.g, color.b};
    if (m_selected_sphere.valid())
    {
        // No shapes on
        if (m_selected_sphere->type == SDFType::OPERATION && m_sdf_tree.children(m_selected_sphere) < 2)
        {
            m_sdf_tree.append(m_selected_sphere, obj);
        }
        else
        {
            m_sdf_tree.append(m_sdf_tree.root(), obj);
        }
    }
    else
    {
        m_sdf_tree.append(m_sdf_tree.root(), obj);
    }

    m_num_spheres++;
    rebuild();
}

void SDFCreator::save(std::filesystem::path path)
{
    std::ofstream file(path, std::ios::binary);
    const unsigned int SERALIZE_VERSION = 1;


    // save version and each object plus its level mount of objects into file
    file << SERALIZE_VERSION;
    for (auto i = m_sdf_tree.prefix_begin(); i != m_sdf_tree.prefix_end(); ++i)
    {
        // save depth
        file << static_cast<int>(m_sdf_tree.depth(i.simplify()));
        file << (*i);
    }
    file.close();
}

void SDFCreator::load(std::filesystem::path path)
{
    if(!std::filesystem::exists(path))
    {
        TraceLog(LOG_ERROR, "File does not exist");
        return;
    }
    std::ifstream file(path, std::ios::binary);
    unsigned int serialize_version = 0;

    if(!file.is_open())
    {
        TraceLog(LOG_ERROR, "Could not open file");
    }
    file.clear();
    file.seekg(std::ios::beg);
    // save version and each object plus its level mount of objects into file
    file >> serialize_version;
    if (serialize_version != 1)
    {
        TraceLog(LOG_ERROR, "File version not supperoted");
    }

    m_sdf_tree.erase();
    //read node
    if(!file.eof())
    {
        int depth = 0;
        SDFObject obj;
        file >> depth;
        file >> obj;
        if(depth != 0)
        {
            TraceLog(LOG_ERROR, "File corupt. No Root exists.");
            return;
        }
        m_sdf_tree.insert(obj);
    }
    int last_depth = 0;
    auto current = m_sdf_tree.root();
    while (!file.eof())
    {
        // save depth
        int depth = 0;
        SDFObject obj;
        file >> depth;
        file >> obj;
        std::cout << "Read" <<obj.index;
        std::cout << ", " << file.tellg() << std::endl;
        if(depth >= last_depth)
        {
            current = m_sdf_tree.append(current, obj);
        }
        else //if(depth < last_depth)
        {
            //walk tree upwards
            for(int i = 0; i <= (last_depth - depth + 1); ++i)
            {
                current = m_sdf_tree.parent(current);
            }
            m_sdf_tree.append(current, obj);
        }

    }
    file.close();
}

bool SDFCreator::loadShader(const Camera& camera, bool field_mode, float blend)
{
    if (m_needs_rebuild)
    {
        rebuildShaders();
    }
    float deltaTime = GetFrameTime();
    m_run_time += deltaTime;

    SetShaderValue(m_main_shader, m_main_locations.view_eye, &camera.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(m_main_shader, m_main_locations.view_center, &camera.target, SHADER_UNIFORM_VEC3);
    SetShaderValue(
        m_main_shader, m_main_locations.resolution,
        (float[2]){(float)GetScreenWidth() * GetWindowScaleDPI().x, (float)GetScreenHeight() * GetWindowScaleDPI().y},
        SHADER_UNIFORM_VEC2);
    SetShaderValue(m_main_shader, m_main_locations.run_time, &m_run_time, SHADER_UNIFORM_FLOAT);
    float mode = static_cast<float>(field_mode);
    SetShaderValue(m_main_shader, m_main_locations.visualizer, &mode, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_main_shader, m_main_locations.blend, &blend, SHADER_UNIFORM_FLOAT);
    loadData();
    if (IsShaderReady(m_main_shader))
    {
        BeginShaderMode(m_main_shader);
        return true;
    }
    return false;
}

void SDFCreator::loadData() const
{
    if (m_selected_sphere.valid())
    {
        const SDFObject* s = &(*m_selected_sphere);
        float used_radius = fmaxf(0.01, fminf(s->corner_radius, fminf(s->size.x, fminf(s->size.y, s->size.z))));
        float data[18] = {
            s->pos.x,
            s->pos.y,
            s->pos.z,

            s->angle.x,
            s->angle.y,
            s->angle.z,

            s->size.x - used_radius,
            s->size.y - used_radius,
            s->size.z - used_radius,

            s->color.r / 255.f,
            s->color.g / 255.f,
            s->color.b / 255.f,

            s->flex_vector.x,
            s->flex_vector.y,
            s->flex_vector.z,

            s->flex_parameter,
            0,
            0,
        };

        SetShaderValueV(m_main_shader, m_main_locations.selected_params, data, SHADER_UNIFORM_VEC3, 6);
    }
}

void SDFCreator::unloadShader()
{
    if (IsShaderReady(m_main_shader))
    {
        EndShaderMode();
    }
}

void SDFCreator::exportObj()
{
    std::string shader_source{};
    shader_source += SHADER_VERSION_PREFIX;
    shader_source += "\nfloat blend = 1;\n";
    shader_source += m_shader_files.getShaderPrefixFS();
    appendMapFunction(shader_source, false, nullptr);
    shader_source += m_shader_files.getSlicerBodyFS();

    Shader slicer_shader = LoadShaderFromMemory(vshader, shader_source.c_str());
    int slicer_z_loc = GetShaderLocation(slicer_shader, "z");

    double startTime = GetTime();
    const float cube_resolution = 0.03;

    BoundingBox bounds = {
        {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()},
        {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()}};

    for (auto i = m_sdf_tree.prefix_begin(); i != m_sdf_tree.prefix_end(); ++i)
    {
        const float radius =
            sqrtf(powf(i->size.x, 2) + powf(i->size.y, 2) + powf(i->size.z, 2));
        bounds.min.x = fminf(bounds.min.x, i->pos.x - radius);
        bounds.min.y = fminf(bounds.min.y, i->pos.y - radius);
        bounds.min.z = fminf(bounds.min.z, i->pos.z - radius);

        bounds.max.x = fmaxf(bounds.max.x, i->pos.x + radius);
        bounds.max.y = fmaxf(bounds.max.y, i->pos.y + radius);
        bounds.max.z = fmaxf(bounds.max.z, i->pos.z + radius);
    }

    // the marching cube sampling lattace must extend beyond the objects you want it to represent
    bounds.min.x -= 1;
    bounds.min.y -= 1;
    bounds.min.z -= 1;

    bounds.max.x += 1;
    bounds.max.y += 1;
    bounds.max.z += 1;

    const int slice_count_x = (int)((bounds.max.x - bounds.min.x) / cube_resolution + 1.5);
    const int slice_count_y = (int)((bounds.max.y - bounds.min.y) / cube_resolution + 1.5);
    const int slice_count_z = (int)((bounds.max.z - bounds.min.z) / cube_resolution + 1.5);

    const float x_step = (bounds.max.x - bounds.min.x) / (slice_count_x - 1);
    const float y_step = (bounds.max.y - bounds.min.y) / (slice_count_y - 1);
    const float z_step = (bounds.max.z - bounds.min.z) / (slice_count_z - 1);

    int data_capacity = 400000000;
    std::string data = "";
    data.resize(data_capacity);
    int data_size = 0;

    RenderTexture2D sliceTexture[2];
    sliceTexture[0] =
        LoadRenderTexture(slice_count_x, slice_count_y); // LoadFloatRenderTexture(slice_count_x, slice_count_y);
    sliceTexture[1] =
        LoadRenderTexture(slice_count_x, slice_count_y); // LoadFloatRenderTexture(slice_count_x, slice_count_y);

    for (int z_index = 0; z_index < slice_count_z - 1; z_index++)
    {
        for (int side = 0; side < 2; side++)
        {
            float z = bounds.min.z + (z_index + side) * z_step;
            SetShaderValue(slicer_shader, slicer_z_loc, &z, SHADER_UNIFORM_FLOAT);
            BeginTextureMode(sliceTexture[side]);
            {
                BeginShaderMode(slicer_shader);
                {
                    rlBegin(RL_QUADS);
                    rlTexCoord2f(bounds.max.x, bounds.min.y);
                    rlVertex2f(0, 0);

                    rlTexCoord2f(bounds.max.x, bounds.max.y);
                    rlVertex2f(0, slice_count_y);

                    rlTexCoord2f(bounds.min.x, bounds.max.y);
                    rlVertex2f(slice_count_x, slice_count_y);

                    rlTexCoord2f(bounds.min.x, bounds.min.y);
                    rlVertex2f(slice_count_x, 0);
                    rlEnd();
                }
                EndShaderMode();
            }
            EndTextureMode();
        }

        float* pixels = reinterpret_cast<float*>(
            rlReadTexturePixels(sliceTexture[0].texture.id, sliceTexture[0].texture.width,
                                sliceTexture[0].texture.height, sliceTexture[0].texture.format));
        float* pixels2 = reinterpret_cast<float*>(
            rlReadTexturePixels(sliceTexture[1].texture.id, sliceTexture[1].texture.width,
                                sliceTexture[1].texture.height, sliceTexture[1].texture.format));

#define SDF_THRESHOLD (0)
        for (int y_index = 0; y_index < slice_count_y - 1; y_index++)
        {
            for (int x_index = 0; x_index < slice_count_x - 1; x_index++)
            {
                float val0 = pixels[(x_index + y_index * slice_count_x) * 1];
                float val1 = pixels[(x_index + 1 + y_index * slice_count_x) * 1];
                float val2 = pixels[(x_index + 1 + (y_index + 1) * slice_count_x) * 1];
                float val3 = pixels[(x_index + (y_index + 1) * slice_count_x) * 1];
                float val4 = pixels2[(x_index + y_index * slice_count_x) * 1];
                float val5 = pixels2[(x_index + 1 + y_index * slice_count_x) * 1];
                float val6 = pixels2[(x_index + 1 + (y_index + 1) * slice_count_x) * 1];
                float val7 = pixels2[(x_index + (y_index + 1) * slice_count_x) * 1];

                Vector4 v0 = {bounds.min.x + x_index * x_step, bounds.min.y + y_index * y_step,
                              bounds.min.z + z_index * z_step, val0};
                Vector4 v1 = {v0.x + x_step, v0.y, v0.z, val1};
                Vector4 v2 = {v0.x + x_step, v0.y + y_step, v0.z, val2};
                Vector4 v3 = {v0.x, v0.y + y_step, v0.z, val3};

                Vector4 v4 = {v0.x, v0.y, v0.z + z_step, val4};
                Vector4 v5 = {v0.x + x_step, v0.y, v0.z + z_step, val5};
                Vector4 v6 = {v0.x + x_step, v0.y + y_step, v0.z + z_step, val6};
                Vector4 v7 = {v0.x, v0.y + y_step, v0.z + z_step, val7};

                int cubeindex = (val0 < SDF_THRESHOLD) << 0 | (val1 < SDF_THRESHOLD) << 1 |
                                (val2 < SDF_THRESHOLD) << 2 | (val3 < SDF_THRESHOLD) << 3 |
                                (val4 < SDF_THRESHOLD) << 4 | (val5 < SDF_THRESHOLD) << 5 |
                                (val6 < SDF_THRESHOLD) << 6 | (val7 < SDF_THRESHOLD) << 7;

                /* Cube is entirely in/out of the surface #1#*/
                if (edgeTable[cubeindex] == 0)
                    continue;

                Vector3 vertlist[12];
                if (edgeTable[cubeindex] & 1)
                    vertlist[0] = VertexInterp(v0, v1, SDF_THRESHOLD);
                if (edgeTable[cubeindex] & 2)
                    vertlist[1] = VertexInterp(v1, v2, SDF_THRESHOLD);
                if (edgeTable[cubeindex] & 4)
                    vertlist[2] = VertexInterp(v2, v3, SDF_THRESHOLD);
                if (edgeTable[cubeindex] & 8)
                    vertlist[3] = VertexInterp(v3, v0, SDF_THRESHOLD);
                if (edgeTable[cubeindex] & 16)
                    vertlist[4] = VertexInterp(v4, v5, SDF_THRESHOLD);
                if (edgeTable[cubeindex] & 32)
                    vertlist[5] = VertexInterp(v5, v6, SDF_THRESHOLD);
                if (edgeTable[cubeindex] & 64)
                    vertlist[6] = VertexInterp(v6, v7, SDF_THRESHOLD);
                if (edgeTable[cubeindex] & 128)
                    vertlist[7] = VertexInterp(v7, v4, SDF_THRESHOLD);
                if (edgeTable[cubeindex] & 256)
                    vertlist[8] = VertexInterp(v0, v4, SDF_THRESHOLD);
                if (edgeTable[cubeindex] & 512)
                    vertlist[9] = VertexInterp(v1, v5, SDF_THRESHOLD);
                if (edgeTable[cubeindex] & 1024)
                    vertlist[10] = VertexInterp(v2, v6, SDF_THRESHOLD);
                if (edgeTable[cubeindex] & 2048)
                    vertlist[11] = VertexInterp(v3, v7, SDF_THRESHOLD);

                for (int i = 0; triTable[cubeindex][i] != -1; i += 3)
                {
                    for (int v = 0; v < 3; v++)
                    {
                        Vector3 pt = vertlist[triTable[cubeindex][i + v]];
                        data += std::format("v {} {} {}\n", pt.x, -pt.y, pt.z);
                    }

                    data += "f -2  -1 -3\n";
                }
            }
        }

        std::free(pixels);
        std::free(pixels2);
    }

    UnloadRenderTexture(sliceTexture[0]);
    UnloadRenderTexture(sliceTexture[1]);

    SaveFileData("export.obj", (void*)(data.c_str()), data_size);

    UnloadShader(slicer_shader);

    double duration = GetTime() - startTime;
    std::cout << std::format("export time {}ms. size: {}MB", duration * 1000, data_size / 1000000.f);
}

void SDFCreator::objectAtPixel(int x, int y, const Camera& camera)
{
    const float start = GetTime();
    std::string shader_source;
    shader_source += SHADER_VERSION_PREFIX;
    shader_source += "\nuniform float blend;\n";
    shader_source += m_shader_files.getShaderPrefixFS();
    appendMapFunction(shader_source, true, nullptr);
    shader_source += m_shader_files.getSelectionFS();
    Shader shader = LoadShaderFromMemory(vshader, shader_source.c_str());
    int eye_loc = GetShaderLocation(shader, "viewEye");
    int center_loc = GetShaderLocation(shader, "viewCenter");
    int resolution_loc = GetShaderLocation(shader, "resolution");
    int blend_loc = GetShaderLocation(shader, "blend\0");
    int count = -1;
    /*glGetProgramiv(shader.id, GL_ACTIVE_UNIFORMS, &count);
     printf("Active Unifroms: %d\n", count);
     for(int i = 0; i < count; ++i)
     {
         int length, size;
         GLenum type;
         char name[20];
         glGetActiveUniform(shader.id, i, 20, &length, &size, &type, name);
         printf("Unifrom #%d Type %u Name %s\n", i, type, name);

     }*/
    SetShaderValue(shader, eye_loc, &camera.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, center_loc, &camera.target, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, resolution_loc, (float[2]){(float)GetScreenWidth(), (float)GetScreenHeight()},
                   SHADER_UNIFORM_VEC2);

    RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    BeginTextureMode(target);
    {
        BeginShaderMode(shader);
        {
            rlBegin(RL_QUADS);
            rlTexCoord2f(x - 1, y - 1);
            rlVertex2f(x - 1, y - 1);

            rlTexCoord2f(x - 1, y + 1);
            rlVertex2f(x - 1, y + 1);

            rlTexCoord2f(x + 1, y + 1);
            rlVertex2f(x + 1, y + 1);

            rlTexCoord2f(x + 1, y - 1);
            rlVertex2f(x + 1, y - 1);
            rlEnd();
        }
        EndShaderMode();
    }
    EndTextureMode();

    uint8_t* pixels = static_cast<uint8_t*>(
        rlReadTexturePixels(target.texture.id, target.texture.width, target.texture.height, target.texture.format));

    int object_index = ((int)pixels[(x + target.texture.width * (target.texture.height - y)) * 4]) - 1;
    std::free(pixels);

    UnloadRenderTexture(target);
    UnloadShader(shader);

    std::cout << "picking object took " << (int)((-start + GetTime()) * 1000) << "ms\n";
    auto old_selected = m_selected_sphere;
    for (auto i = m_sdf_tree.prefix_begin(); i != m_sdf_tree.prefix_end(); i++)
    {
        if (i->index == (object_index - 1))
        {
            m_selected_sphere = i.simplify();
        }
    }

    if (m_selected_sphere != old_selected)
    {
        rebuild();
    }

    std::cout << object_index << std::endl;
}
bool SDFCreator::isSelected() const
{
    return m_selected_sphere.valid();
}

void SDFCreator::select(const SDFIterator& i)
{
    auto old = m_selected_sphere;
    m_selected_sphere = i;
    if (old != m_selected_sphere)
    {
        rebuild();
    }
}

std::optional<SDFIterator> SDFCreator::getSelected()
{
    if (m_selected_sphere.valid())
    {
        return m_selected_sphere;
    }
    return std::nullopt;
}
void SDFCreator::clear()
{
    constexpr SDFObject root;
    m_sdf_tree.insert(root);
}

void SDFCreator::appendMapFunction(std::string& result, bool use_color_as_index, SDFObject* dynamic_index)
{
    // reset all sdf creation
    for (auto i = m_sdf_tree.prefix_begin(); i != m_sdf_tree.prefix_end(); ++i)
    {
        i->done = false;
    }
    std::string map;
    map += "uniform vec3 selectionValues[6];\n\n";
    map += "vec4 signed_distance_field(vec3 pos ){\n"
           "\tvec4 distance = vec4(999999., 0, 0, 0);\n";
    int index_counter = 0;
    std::stringstream shader_content;

    for (auto i = m_sdf_tree.prefix_begin(); i != m_sdf_tree.prefix_end(); ++i)
    {
        generateShaderContent(shader_content, i.simplify(), index_counter, use_color_as_index, dynamic_index);
    }
    map += shader_content.str();
    map += "\treturn distance;\n}\n";

#if DEMO_VIDEO_FEATURES
    SaveFileText("map.glsl", map);
#endif

    result += map;
}

void SDFCreator::generateShaderObject(std::stringstream& content, SDFIterator current, int& count,
                                      bool use_color_as_index, const SDFObject* dynamic_index, bool with_parent)
{
    if (current->object_type == SDFObjectType::NONE)
    {
        return;
    }

    // save index for selecting
    // const std::string symmetry[8] = {"", "opSymX", "opSymY", "opSymXY", "opSymZ", "opSymXZ", "opSymYZ", "opSymXYZ"};
    // float used_radius = fmaxf(0.01, fminf(corner_radius, fminf(size.x, fminf(size.y, size.z))));
    // int mirror_index = (mirror.z << 2) | (mirror.y << 1) | mirror.x;
    if (!with_parent)
    {
        content << "distance = ";
    }

    uint8_t r = use_color_as_index ? (uint8_t)(count + 1) : current->color.r;
    uint8_t g = use_color_as_index ? (uint8_t)(count >> 8) : current->color.g;
    uint8_t b = use_color_as_index ? (uint8_t)(count >> 16) : current->color.b;

    content << "\tMin(\n\t\tvec4(\n";
    switch (current->object_type)
    {
        case SDFObjectType::NONE:

            break;
        case SDFObjectType::ROUND_BOX:
            content << "sdRoundBox(";
            break;
        case SDFObjectType::SPHERE:
            content << "sdSphere(";
            break;
        case SDFObjectType::BOX_FRAME:
            content << "sdBoxFrame(";
            break;
        case SDFObjectType::TORUS:
            content << "sdTorus(";
            break;
        case SDFObjectType::BOX:
            content << "sdBox(";
            break;
        case SDFObjectType::CAPPED_TORUS:
            content << "sdCappedTorus(";
            break;
        case SDFObjectType::LINK:
            content << "sdLink(";
            break;
        case SDFObjectType::CYLINDER:
            content << "sdCylinder(";
            break;
        case SDFObjectType::CONE:
            content << "sdCone(";
            break;
        case SDFObjectType::PLANE:
            content << "sdPlane(";
            break;
        case SDFObjectType::HEX_PRISM:
            content << "sdHexPrism(";
            break;
        case SDFObjectType::TRI_PRISM:
            content << "sdTriPrism(";
            break;
        case SDFObjectType::CAPSULE:
            content << "sdCapsule(";
            break;
        case SDFObjectType::VERTICAL_CAPSULE:
            content << "sdVerticalCapsule(";
            break;
        case SDFObjectType::CAPPED_CYLINDER:
            content << "sdCappedCylinder(";
            break;
        case SDFObjectType::ROUNDED_CYLINDER:
            content << "sdRoundedCylinder(";
            break;
        case SDFObjectType::CAPPED_CONE:
            content << "sdCappedCone(";
            break;
        case SDFObjectType::SOLDID_ANGLE:
            content << "sdSolidAngle(";
            break;
        case SDFObjectType::CUT_SPHERE:
            content << "sdCutSphere(";
            break;
        case SDFObjectType::CUT_HOLLOW_SPHERE:
            content << "sdCutHollowSphere(";
            break;
        case SDFObjectType::DEATH_STAR:
            content << "sdDeathStar(";
            break;
        case SDFObjectType::ROUND_CONE:
            content << "sdRoundCone(";
            break;
        case SDFObjectType::ELLIPSOID:
            content << "sdEllipsoid(";
            break;
        case SDFObjectType::VESICA_SEGMENT:
            content << "sdVesicaSegment(";
            break;
        case SDFObjectType::RHOMBUS:
            content << "sdRhombus(";
            break;
        case SDFObjectType::OCTAHEDRON:
            content << "sdOctahedron(";
            break;
        case SDFObjectType::PYRAMID:
            content << "sdPyramid(";
            break;
    }
    if (&(*current) == dynamic_index)
    {
        content << std::string("\t\topRotateXYZ((pos) - selectionValues[0], // position\n"
                               "\t\tselectionValues[1]), // angle\n"
                               "\t\tselectionValues[4], //extra vector\n"
                               "\t\tselectionValues[2],  // size\n"
                               "\t\tselectionValues[5].x), // extra  parameter\n"
                               "\t\tselectionValues[3]), // color\n"
                               "\t\tdistance\n");
    }
    else
    {
        // Rotation Matrix
        //  const bool rotated = fabsf(current->angle.x) > .01 || fabsf(current->angle.y) > 0.01 ||
        //  fabsf(current->angle.z) > .01; if (rotated)
        //  {
        float cz = cos(current->angle.z);
        float sz = sin(current->angle.z);
        float cy = cos(current->angle.y);
        float sy = sin(current->angle.y);
        float cx = cos(current->angle.x);
        float sx = sin(current->angle.x);

        content << std::format("\t\t\tmat3({}, {}, {},"
                               "{}, {}, {},"
                               "{}, {}, {})*\n",
                               cz * cy, cz * sy * sx - cx * sz, sz * sx + cz * cx * sy,

                               cy * sz, cz * cx + sz * sy * sx, cx * sz * sy - cz * sx,

                               -sy, cy * sx, cy * cx);
        // }

        // position vector is the same everywhere
        content << std::format("\t\t\t\t((pos) - vec3({},{},{})), // position\n", current->pos.x, current->pos.y,
                               current->pos.z);

        content << std::format("\t\t\tvec3({},{},{}),// mostly unused\n"
                               "\t\t\tvec3({},{},{}),// size\n"
                               "\t\t\t{}), // felxible parameter\n"
                               "\t\t\t{},{},{}), // color\n"
                               "\t\tdistance",
                               current->flex_vector.x, current->flex_vector.y, current->flex_vector.z, current->size.x,
                               current->size.y, current->size.z, current->flex_parameter, r / 255.f, g / 255.f,
                               b / 255.f);
    }
    if (!with_parent)
    {
        content << ");\n";
    }
    else
    {
        content << ")";
    }
}
void SDFCreator::generateShaderOperation(std::stringstream& content, SDFIterator current, int count,
                                         bool use_color_as_index, const SDFObject* dynamic_index, bool with_parent)
{
    if (current->operation_type == SDFOperationType::NONE)
    {
        return;
    }
    if (m_sdf_tree.children(current) == 2)
    {
        if (with_parent)
        {
            content << "Min(";
        }
        else
        {
            content << "distance = Min(";
        }
        switch (current->operation_type)
        {
            default:
                TraceLog(LOG_ERROR, "Cant perform operation with this count of childs %i",
                         m_sdf_tree.children(current));
                break;
            case SDFOperationType::NONE:
                break;
            case SDFOperationType::SMOOTH_UNION:
                content << "opSmoothUnion(";
                break;
            case SDFOperationType::SMOOTH_SUBSTRACTION:
                content << "opSmoothSubtraction(";
                break;
            case SDFOperationType::SMOOTH_INTERSECTION:
                content << "opSmoothSubtraction(";
                break;
            case SDFOperationType::UNION:
                content << "opUnion(";
                break;
            case SDFOperationType::SUBSTRACTION:
                content << "opSubtraction(";
                break;
            case SDFOperationType::INTERSECTION:
                content << "opIntersection(";
                break;
        }
        generateShaderContent(content, m_sdf_tree.child(current, 0), count, use_color_as_index, dynamic_index, true);
        content << ", //end first operand \n";
        generateShaderContent(content, m_sdf_tree.child(current, 1), count, use_color_as_index, dynamic_index, true);
        if (with_parent)
        {
            content << "), distance) //compare to old distance\n";
        }
        else
        {
            content << "), distance); //compare to old distance\n";
        }
    }
    if (m_sdf_tree.children(current) == 1)
    {
        if (current->operation_type != SDFOperationType::ROUND || current->operation_type != SDFOperationType::ONION)
        {
            return;
        }

        if (with_parent)
        {
            content << "Min(";
        }
        else
        {
            content << "distance = Min(";
        }
        switch (current->operation_type)
        {
            default:
                TraceLog(LOG_ERROR, "Cant perform operation with this count of childs %i",
                         m_sdf_tree.children(current));
                return;
            case SDFOperationType::ROUND:
                content << "opRound(";
                break;
            case SDFOperationType::ONION:
                content << "opOnion(";
                break;
        }
        generateShaderContent(content, m_sdf_tree.child(current, 0), count, use_color_as_index, dynamic_index, true);
        content << ", //end first  operand\n";
        content << std::to_string(current->flex_parameter);
        if (with_parent)
        {
            content << "), distance) //compare to old distance\n";
        }
        else
        {
            content << "), distance); //compare to old distance\n";
        }
    }
}

void SDFCreator::generateShaderContent(std::stringstream& content, SDFIterator current, int& count,
                                       bool use_color_as_index, const SDFObject* dynamic_index, bool with_parent)
{
    current->index = count;
    count++;
    if (current->done)
        return;
    current->done = true;
    switch (current->type)
    {
        case SDFType::OBJECT:
            generateShaderObject(content, current, count, use_color_as_index, dynamic_index, with_parent);
            break;
        case SDFType::OPERATION:
            generateShaderOperation(content, current, count, use_color_as_index, dynamic_index, with_parent);
            break;
    }
}

stlplus::ntree<SDFObject>& SDFCreator::getTree()
{
    return m_sdf_tree;
}
