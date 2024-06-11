//
// Created by Daniel Hagen on 10.06.2024.
//
#include "SDFCreator.h"
#include "raylib.h"
#include "rlgl.h"
#include <cassert>
#include "raymath.h"

#include <iostream>

SDFCreator::SDFCreator()
{
    m_last_save = GetTime();
    m_shader_files.loadShader();
    addShape({0, 255, 0});
    m_selected_sphere = -1;
}
void SDFCreator::rebuildShaders()
{
    m_needs_rebuild = false;
    UnloadShader(m_main_shader);

    std::string map_function = "";
    appendMapFunction(map_function, false, m_selected_sphere);

    std::string result = "";
    result += SHADER_VERSION_PREFIX "out vec4 finalColor;\n"
           "uniform vec3 viewEye; \n"
           "uniform vec3 viewCenter; \n"
           "uniform float runTime; \n"
           "uniform float visualizer; \n"
           "uniform vec2 resolution;\n";
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
}

void SDFCreator::deleteSphere()
{
    // memmove(&spheres[index], &spheres[index + 1], sizeof(Sphere) * (m_num_spheres - index));
    m_objects.erase(m_objects.begin() + m_selected_sphere);
    m_num_spheres--;

    // if (m_selected_sphere == index)
    // {
    //     m_selected_sphere = m_num_spheres - 1;
    // }
    // else if (m_selected_sphere > index)
    // {
    //     m_selected_sphere--;
    // }

    m_needs_rebuild = true;
}

void SDFCreator::addShape(Color color)
{
    /*spheres[m_num_spheres] = (Sphere){
        .size = {1, 1, 1},
        .color =
            {
            last_color_set.r,
            last_color_set.g,
            last_color_set.b,
        }};*/
    SDFObject obj{};
    obj.size = {1, 1, 1};
    obj.color = {color.r, color.g, color.b};
    m_objects.push_back(obj);
    m_selected_sphere = m_num_spheres;
    m_num_spheres++;
    m_needs_rebuild = true;
}

void SDFCreator::save(std::filesystem::path path)
{
    // save data and amount of objects into file
    const int size = sizeof(int) + sizeof(SDFObject) * m_num_spheres;
    char* data = static_cast<char*>(std::malloc(size));
    *(int*)(void*)data = m_num_spheres;
    memcpy(data + sizeof(int), m_objects.data(), m_num_spheres * sizeof(SDFObject));

    SaveFileData(path.string().c_str(), data, size);

    std::free(data);
    m_last_save = GetTime();
}

void SDFCreator::openSnapshot(std::filesystem::path path)
{
    int size;
    unsigned char* data = LoadFileData(path.string().c_str(), &size);

    assert(data);

    m_num_spheres = *(int*)(void*)data;
    memcpy(m_objects.data(), data + sizeof(int), sizeof(SDFObject) * m_num_spheres);

    UnloadFileData(data);

    m_selected_sphere = -1;
    m_needs_rebuild = true;
    m_last_save = GetTime();
}

bool SDFCreator::loadShader(const Camera& camera)
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
    float mode = visuals_mode;
    SetShaderValue(m_main_shader, m_main_locations.visualizer, &mode, SHADER_UNIFORM_FLOAT);
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
    if (m_selected_sphere >= 0 && m_selected_sphere < m_objects.size())
    {
        const SDFObject* s = &m_objects.at(m_selected_sphere);
        float used_radius = fmaxf(0.01, fminf(s->corner_radius, fminf(s->size.x, fminf(s->size.y, s->size.z))));
        float data[15] = {
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

            used_radius,
            fmaxf(s->blob_amount, 0.0001),
            0,
        };

        SetShaderValueV(m_main_shader, m_main_locations.selected_params, data, SHADER_UNIFORM_VEC3, 5);
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
    shader_source += m_shader_files.getShaderPrefixFS();
    appendMapFunction(shader_source, false, -1);
    shader_source += m_shader_files.getSlicerBodyFS();

    Shader slicer_shader = LoadShaderFromMemory(vshader, shader_source.c_str());
    int slicer_z_loc = GetShaderLocation(slicer_shader, "z");

    double startTime = GetTime();
    const float cube_resolution = 0.03;

    BoundingBox bounds = {
        {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()},
        {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()}};

    for (int i = 0; i < m_objects.size(); i++)
    {
        const float radius =
            sqrtf(powf(m_objects[i].size.x, 2) + powf(m_objects[i].size.y, 2) + powf(m_objects[i].size.z, 2));
        bounds.min.x = fminf(bounds.min.x, m_objects[i].pos.x - radius);
        bounds.min.y = fminf(bounds.min.y, m_objects[i].pos.y - radius);
        bounds.min.z = fminf(bounds.min.z, m_objects[i].pos.z - radius);

        bounds.max.x = fmaxf(bounds.max.x, m_objects[i].pos.x + radius);
        bounds.max.y = fmaxf(bounds.max.y, m_objects[i].pos.y + radius);
        bounds.max.z = fmaxf(bounds.max.z, m_objects[i].pos.z + radius);
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

                /* Cube is entirely in/out of the surface */
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
    std::string shader_source = "";
    shader_source += SHADER_VERSION_PREFIX;
    shader_source += m_shader_files.getShaderPrefixFS();
    appendMapFunction(shader_source, true, -1);
    shader_source += m_shader_files.getSelectionFS();

    Shader shader = LoadShaderFromMemory(vshader, shader_source.c_str());

    int eye_loc = GetShaderLocation(shader, "viewEye");
    int center_loc = GetShaderLocation(shader, "viewCenter");
    int resolution_loc = GetShaderLocation(shader, "resolution");

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

    if (object_index >= 0 && object_index < m_objects.size())
    {
        m_selected_sphere = object_index;
    }
}
bool SDFCreator::isSelected() const
{
    if (m_selected_sphere >= 0 && m_selected_sphere < m_objects.size())
    {
        return true;
    }
    else
    {
        return false;
    }
}
std::optional<SDFObject*> SDFCreator::getSelected()
{
    if (m_selected_sphere >= 0 && m_selected_sphere < m_objects.size())
    {
        return &m_objects.at(m_selected_sphere);
    }
    return std::nullopt;
}

Vector3 VertexInterp(Vector4 p1, Vector4 p2, float threshold)
{
    if (fabsf(threshold - p1.w) < 0.00001)
        return *(Vector3*)&p1;
    if (fabsf(threshold - p2.w) < 0.00001)
        return *(Vector3*)&p2;
    if (fabsf(p1.w - p2.w) < 0.00001)
        return *(Vector3*)&p1;

    float mu = (threshold - p1.w) / (p2.w - p1.w);
    Vector3 r = {
        p1.x + mu * (p2.x - p1.x),
        p1.y + mu * (p2.y - p1.y),
        p1.z + mu * (p2.z - p1.z),
    };

    return r;
}

int SDFCreator::RayPlaneIntersection(const Vector3 RayOrigin, const Vector3 RayDirection, const Vector3 PlanePoint,
                                     const Vector3 PlaneNormal, Vector3* IntersectionPoint)
{
    float dotProduct =
        (PlaneNormal.x * RayDirection.x) + (PlaneNormal.y * RayDirection.y) + (PlaneNormal.z * RayDirection.z);

    // Check if the ray is parallel to the plane
    if (dotProduct == 0.0f)
    {
        return 0;
    }

    float t = ((PlanePoint.x - RayOrigin.x) * PlaneNormal.x + (PlanePoint.y - RayOrigin.y) * PlaneNormal.y +
               (PlanePoint.z - RayOrigin.z) * PlaneNormal.z) /
              dotProduct;

    IntersectionPoint->x = RayOrigin.x + t * RayDirection.x;
    IntersectionPoint->y = RayOrigin.y + t * RayDirection.y;
    IntersectionPoint->z = RayOrigin.z + t * RayDirection.z;

    return 1;
}

Vector3 SDFCreator::WorldToCamera(Vector3 worldPos, Matrix cameraMatrix)
{
    return Vector3Transform(worldPos, cameraMatrix);
}

Vector3 SDFCreator::CameraToWorld(Vector3 worldPos, Matrix cameraMatrix)
{
    return Vector3Transform(worldPos, MatrixInvert(cameraMatrix));
}

Vector3 SDFCreator::VectorProjection(const Vector3 vectorToProject, const Vector3 targetVector)
{
    float dotProduct = (vectorToProject.x * targetVector.x) + (vectorToProject.y * targetVector.y) +
                       (vectorToProject.z * targetVector.z);

    float targetVectorLengthSquared =
        (targetVector.x * targetVector.x) + (targetVector.y * targetVector.y) + (targetVector.z * targetVector.z);

    float scale = dotProduct / targetVectorLengthSquared;

    Vector3 projection;
    projection.x = targetVector.x * scale;
    projection.y = targetVector.y * scale;
    projection.z = targetVector.z * scale;

    return projection;
}

// Find the point on line p1 to p2 nearest to line p2 to p4
Vector3 SDFCreator::NearestPointOnLine(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4)
{
    float mua;

    Vector3 p13, p43, p21;
    float d1343, d4321, d1321, d4343, d2121;
    float numer, denom;

    const float EPS = 0.001;

    p13.x = p1.x - p3.x;
    p13.y = p1.y - p3.y;
    p13.z = p1.z - p3.z;
    p43.x = p4.x - p3.x;
    p43.y = p4.y - p3.y;
    p43.z = p4.z - p3.z;
    if (fabs(p43.x) < EPS && fabs(p43.y) < EPS && fabs(p43.z) < EPS)
        return (Vector3){};
    p21.x = p2.x - p1.x;
    p21.y = p2.y - p1.y;
    p21.z = p2.z - p1.z;
    if (fabs(p21.x) < EPS && fabs(p21.y) < EPS && fabs(p21.z) < EPS)
        return (Vector3){};

    d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
    d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
    d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
    d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
    d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

    denom = d2121 * d4343 - d4321 * d4321;
    if (fabs(denom) < EPS)
        return (Vector3){};
    numer = d1343 * d4321 - d1321 * d4343;

    mua = numer / denom;

    return (Vector3){p1.x + mua * p21.x, p1.y + mua * p21.y, p1.z + mua * p21.z};
}

BoundingBox SDFCreator::boundingBoxSized(Vector3 center, float size)
{
    return (BoundingBox){
        Vector3SubtractValue(center, size / 2),
        Vector3AddValue(center, size / 2),
    };
}

BoundingBox SDFCreator::shapeBoundingBox(SDFObject s)
{
    // const float radius = sqrtf(powf(s.size.x, 2) + powf(s.size.y, 2) + powf(s.size.z, 2));
    return (BoundingBox){
        Vector3Subtract(s.pos, s.size),
        Vector3Add(s.pos, s.size),
    };
}

Vector3 SDFCreator::VertexInterp(Vector4 p1, Vector4 p2, float threshold)
{
    if (fabsf(threshold - p1.w) < 0.00001)
        return *(Vector3*)&p1;
    if (fabsf(threshold - p2.w) < 0.00001)
        return *(Vector3*)&p2;
    if (fabsf(p1.w - p2.w) < 0.00001)
        return *(Vector3*)&p1;

    float mu = (threshold - p1.w) / (p2.w - p1.w);
    Vector3 r = {
        p1.x + mu * (p2.x - p1.x),
        p1.y + mu * (p2.y - p1.y),
        p1.z + mu * (p2.z - p1.z),
    };

    return r;
}

void SDFCreator::appendMapFunction(std::string& result, bool use_color_as_index, int dynamic_index) const
{
    std::string map = "";

    map += "uniform vec3 selectionValues[5];\n\n";

#if DEMO_VIDEO_FEATURES
    if (false_color_mode)
    {
        map += "#define FALSE_COLOR_MODE 1\n\n";
    }
#endif

    map += "vec4 signed_distance_field( in vec3 pos ){\n"
           "\tvec4 distance = vec4(999999.,0,0,0);\n";
    const std::string symmetry[8] = {"", "opSymX", "opSymY", "opSymXY", "opSymZ", "opSymXZ", "opSymYZ", "opSymXYZ"};
    for (int i = 0; i < m_objects.size(); i++)
    {
        SDFObject s = m_objects[i];
        float used_radius = fmaxf(0.01, fminf(s.corner_radius, fminf(s.size.x, fminf(s.size.y, s.size.z))));

        int mirror_index = (s.mirror.z << 2) | (s.mirror.y << 1) | s.mirror.x;
        if (i == dynamic_index)
        {
            map += std::format("\tdistance = {}(\n"
                              "\t\tvec4(RoundBox(\n"
                              "\t\t\t\topRotateXYZ(\n"
                              "\t\t\t\t\t{}(pos) - selectionValues[0], // position\n"
                              "\t\t\t\t\tselectionValues[1]), // angle\n"
                              "\t\t\t\tselectionValues[2],  // size\n"
                              "\t\t\t\tselectionValues[4].x), // corner radius\n"
                              "\t\t\tselectionValues[3]), // color\n"
                              "\t\tdistance,\n\t\tselectionValues[4].y); // blobbyness\n",
                              s.subtract ? "opSmoothSubtraction" : "BlobbyMin", symmetry[mirror_index]);
        }
        else
        {
            uint8_t r = use_color_as_index ? (uint8_t)(i + 1) : s.color.r;
            uint8_t g = use_color_as_index ? 0 : s.color.g;
            uint8_t b = use_color_as_index ? 0 : s.color.b;

#if DEMO_VIDEO_FEATURES
            if (false_color_mode)
            {
                Color c = ColorFromHSV((i * 97) % 360, 1, 0.5);
                r = c.r;
                g = c.g;
                b = c.b;
            }
#endif

            const std::string opName = s.subtract ? "opSmoothSubtraction"
                                                  : ((use_color_as_index
#if DEMO_VIDEO_FEATURES
                                                      || false_color_mode
#endif
                                                      )
                                                         ? "opSmoothUnionSteppedColor"
                                                         : (s.blob_amount > 0 ? "BlobbyMin" : "Min"));
            map += std::format("\tdistance = {}(\n\t\tvec4(RoundBox(\n", opName);

            const bool rotated = fabsf(s.angle.x) > .01 || fabsf(s.angle.y) > 0.01 || fabsf(s.angle.z) > .01;
            if (rotated)
            {
                float cz = cos(s.angle.z);
                float sz = sin(s.angle.z);
                float cy = cos(s.angle.y);
                float sy = sin(s.angle.y);
                float cx = cos(s.angle.x);
                float sx = sin(s.angle.x);

                map += std::format("\t\t\tmat3({}, {}, {},"
                                  "{}, {}, {},"
                                  "{}, {}, {})*\n",
                                  cz * cy, cz * sy * sx - cx * sz, sz * sx + cz * cx * sy,

                                  cy * sz, cz * cx + sz * sy * sx, cx * sz * sy - cz * sx,

                                  -sy, cy * sx, cy * cx);
            }

            map += std::format("\t\t\t\t({}(pos) - vec3({},{},{})), // position\n", symmetry[mirror_index], s.pos.x,
                               s.pos.y, s.pos.z);

            map += std::format("\t\t\tvec3({},{},{}),// size\n"
                               "\t\t\t{}), // corner radius\n"
                               "\t\t\t{},{},{}), // color\n"
                               "\t\tdistance",
                               s.size.x - used_radius, s.size.y - used_radius, s.size.z - used_radius, used_radius,
                               r / 255.f, g / 255.f, b / 255.f);

            if (opName == "Min")
            {
                map += ");\n";
            }
            else
            {
                map += std::format(",\n\t\t{});  // blobbyness\n", fmaxf(s.blob_amount, 0.0001));
            }
        }
    }

    map += "\treturn distance;\n}\n";

#if DEMO_VIDEO_FEATURES
    SaveFileText("map.glsl", map);
#endif

    result += map;
}
