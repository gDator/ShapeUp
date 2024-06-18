//
// Created by Daniel Hagen on 10.06.2024.
//
#include "SDFCreatorGui.h"
#include "raylib.h"
#include "imgui.h"
#include "imgui_spectrum.h"
#include "rlImGui.h"
#include <imgui_internal.h>
#include "rcamera.h"
#include "raymath.h"
#include "imgui_impl_raylib.h"
#include "SDFUtil.h"

SDFCreatorGui::SDFCreatorGui()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(m_screen_width, m_screen_height, "ShapeUp");
    SetTargetFPS(144);
    rlImGuiSetup(true);
    ImGuiIO io = ImGui::GetIO();
    ImGui::GetIO().Fonts->Clear();
    ImGui::Spectrum::LoadFont();
    ImGui::Spectrum::StyleColorsSpectrumDark();
    Imgui_ImplRaylib_BuildFontAtlas();

    m_texture_camera.zoom = 1;
    m_texture_camera.target.x = 0;
    m_texture_camera.target.y = 0;
    m_texture_camera.rotation = 0;
    m_texture_camera.offset.x = 0;
    m_texture_camera.offset.y = 0;

    m_camera.position = (Vector3){2.5f, 2.5f, 3.0f};
    m_camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    m_camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    m_camera.fovy = 55.0f;
    m_camera.projection = CAMERA_PERSPECTIVE;

    m_texture = LoadRenderTexture(10, 10);

    SetExitKey(KEY_NULL); // Dont exit on KEY_ESCAPE
}
void SDFCreatorGui::run()
{
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        // BeginTextureMode(m_texture);
        ClearBackground(DARKGRAY);

        // m_texture_camera with our view offset with a world origin of 0,0
        // BeginMode2D(m_texture_camera);

        if (m_sdf_creator.loadShader(m_camera, m_show_sdf_field, m_blend))
        {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), DARKBLUE);
            m_sdf_creator.unloadShader();
        }
        drawManipulator();
        // EndMode2D();
        // EndTextureMode();
        // start ImGui Conent
        rlImGuiBegin();
        drawGui();
        DrawFPS(0, 0);
        // end ImGui Content
        rlImGuiEnd();

        EndDrawing();
        handleInput();
        //----------------------------------------------------------------------------------
    }
    rlImGuiShutdown();
    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
}
void SDFCreatorGui::drawGui()
{
    static float export_resolution = 0.01;
    /*if (ImGui::Begin("View"))
    {
        ImVec2 size = ImGui::GetWindowSize();
        size.y -= (ImGui::GetCurrentWindow()->TitleBarHeight());
        if (size.x != m_texture.texture.width || size.y != m_texture.texture.height)
        {
            UnloadRenderTexture(m_texture);
            m_texture = LoadRenderTexture(size.x, size.y);
        }
        rlImGuiImageRenderTexture(&m_texture);
    }
    ImGui::End();*/

    static float last_color[3] = {1, 1, 1};
    if (ImGui::Begin("Edit"))
    {
        m_edit = ImGui::IsWindowFocused();
        if (ImGui::Button("Save"))
        {
            m_sdf_creator.save("./file.shapeup");
        }
        ImGui::SameLine();
        if (ImGui::Button("Load"))
        {
            m_sdf_creator.load("./file.shapeup");
        }
        ImGui::SameLine();
        if (ImGui::Button("Export"))
        {
            m_sdf_creator.exportPly("./export.ply", export_resolution);
        }
        ImGui::InputFloat("Export Resolution", &export_resolution, 0.0001, 1);
        if (ImGui::Button("Clear"))
        {
            m_sdf_creator.clear();
        }
        if (ImGui::Button("Add"))
        {
            m_sdf_creator.addShape({255, 255, 255});
        }
        if (ImGui::Button("Delete"))
        {
            m_sdf_creator.deleteObject();
        }
        if (ImGui::BeginListBox("Structure"))
        {
            std::string object_string{};
            for (auto i = m_sdf_creator.getTree().prefix_begin(); i != m_sdf_creator.getTree().prefix_end(); ++i)
            {
                object_string = std::string(m_sdf_creator.getTree().depth(i.simplify()), '-');
                object_string += ObjectToString(*i);
                bool selected = false;
                if (m_sdf_creator.isSelected())
                {
                    selected = &(*m_sdf_creator.getSelected().value()) == &(*i.simplify());
                }
                if (ImGui::Selectable(object_string.c_str(), selected))
                {
                    m_sdf_creator.select(i.simplify());
                }
            }
            ImGui::EndListBox();
        }
        ImGui::Checkbox("SDF Field", &m_show_sdf_field);
        ImGui::DragFloat("SDF Blend", &m_blend);
        if (m_sdf_creator.isSelected())
        {
            SDFObject* old = &(*m_sdf_creator.getSelected().value());
            if (ImGui::BeginCombo("Type", types_to_string.at(old->type).c_str()))
            {
                if (ImGui::Selectable("Object", (old->type == SDFType::OBJECT)))
                {
                    old->type = SDFType::OBJECT;
                    m_sdf_creator.rebuild();
                }
                if (ImGui::Selectable("Operation", (old->type == SDFType::OPERATION)))
                {
                    old->type = SDFType::OPERATION;
                    m_sdf_creator.rebuild();
                }
                ImGui::EndCombo();
            }

            if (old->type == SDFType::OBJECT)
            {
                if (ImGui::BeginCombo("Object", object_to_string.at(old->object_type).c_str()))
                {
                    for (auto&& [o_type, description] : object_types)
                    {
                        if (ImGui::Selectable(description.c_str(), (old->object_type == o_type)))
                        {
                            old->object_type = o_type;
                            m_sdf_creator.rebuild();
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            else
            {
                if (ImGui::BeginCombo("Operation", operation_to_string.at(old->operation_type).c_str()))
                {
                    for (auto&& [o_type, description] : operation_types)
                    {
                        if (ImGui::Selectable(description.c_str(), (old->operation_type == o_type)))
                        {
                            old->operation_type = o_type;
                            m_sdf_creator.rebuild();
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            ImGui::DragFloat3("Position", &old->pos.x);
            ImGui::DragFloat3("Scale", &old->size.x);
            ImGui::DragFloat3("Rotation", &old->angle.x);
            ImGui::DragFloat3("Flexible Parameters", &old->flex_vector.x);
            if (ImGui::ColorPicker3("Color", last_color))
            {
                old->color.r = last_color[0] * 255;
                old->color.g = last_color[1] * 255;
                old->color.b = last_color[2] * 255;
            }
            ImGui::SliderFloat("Extra Parameter", &old->flex_parameter, 0, 10);
            // ImGui::SliderFloat("Roundness", &old->corner_radius, 0, 1);
            // ImGui::Checkbox("Cut out", &old->subtract);
            // ImGui::Text("Mirror");
            // ImGui::Checkbox("X", &old->mirror.x);
            // ImGui::SameLine();
            // ImGui::Checkbox("Y", &old->mirror.y);
            // ImGui::SameLine();
            // ImGui::Checkbox("Z", &old->mirror.z);
        }
        ImGui::End();

        if (m_focused_control != Control::CONTROL_NONE)
        {
            ImGui::Text("Nudge Value: Up & Down Arrows    Cancel: Escape    Done: Enter");
        }
        else if (m_mouse_action == Control::CONTROL_NONE)
        {
            DrawText("Add Shape: A    Delete: X    Grab: G    Rotate: R    Scale: S    Camera: Click+Drag", 8, 11, 10,
                     {255, 255, 255, 255});
        }
        else if (m_mouse_action == Control::CONTROL_TRANSLATE || m_mouse_action == Control::CONTROL_ROTATE ||
                 m_mouse_action == Control::CONTROL_SCALE)
        {
            DrawText("Change axis: X Y Z    Cancel: Escape    Done: Enter", 8, 11, 10, {255, 255, 255, 255});
        }
        else if (m_mouse_action == Control::CONTROL_ROTATE_CAMERA)
        {
            DrawText("Pan: Alt+Drag", 8, 11, 10, {255, 255, 255, 255});
        }
    }
}
void SDFCreatorGui::handleInput()
{
    Ray ray = GetMouseRay(GetMousePosition(), m_camera);
    static SDFObject before_edit;
    if (m_focused_control == Control::CONTROL_NONE)
    {
        if (m_mouse_action == Control::CONTROL_NONE && m_sdf_creator.isSelected() &&
            (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_X)))
        {
            m_sdf_creator.deleteObject();
        }

        if (m_sdf_creator.isSelected() && IsKeyPressed(KEY_D) &&
            (IsKeyDown(KEY_RIGHT_SUPER) || IsKeyDown(KEY_LEFT_SUPER)))
        {
            // TODO: ??
            // spheres[num_spheres] = m_sdf_creator.getSelected();
            // selected_sphere = num_spheres;
            // num_spheres++;
            m_sdf_creator.rebuild(); // needs_rebuild = true;
        }

        if (IsKeyPressed(KEY_A))
        {
            m_sdf_creator.addShape({255, 255, 255});
        }

        // add shape
        if (m_sdf_creator.isSelected() && IsKeyPressed(KEY_G))
        {
            m_mouse_action = Control::CONTROL_TRANSLATE;
            m_last_axis_set = 0;
            m_controlled_axis.mask = 0x7;
            before_edit = *m_sdf_creator.getSelected().value();
        }
        if (m_sdf_creator.isSelected() && IsKeyPressed(KEY_R))
        {
            m_mouse_action = Control::CONTROL_ROTATE;
            m_last_axis_set = 0;
            m_controlled_axis.mask = 0x7;
            before_edit = *m_sdf_creator.getSelected().value();
        }
        if (m_sdf_creator.isSelected() && IsKeyPressed(KEY_S))
        {
            m_mouse_action = Control::CONTROL_SCALE;
            m_last_axis_set = 0;
            m_controlled_axis.mask = 0x7;
            before_edit = *m_sdf_creator.getSelected().value();
        }
    }

    if (m_sdf_creator.isSelected() &&
        (m_mouse_action == Control::CONTROL_TRANSLATE || m_mouse_action == Control::CONTROL_ROTATE ||
         m_mouse_action == Control::CONTROL_SCALE))
    {
        bool should_set = GetTime() - m_last_axis_set > 1;
        if (IsKeyPressed(KEY_X))
        {
            m_controlled_axis.mask = (should_set || !(m_controlled_axis.mask ^ 1)) ? 1 : (m_controlled_axis.mask ^ 1);
            m_last_axis_set = GetTime();
            *m_sdf_creator.getSelected().value() = before_edit;
        }
        if (IsKeyPressed(KEY_Y))
        {
            m_controlled_axis.mask = (should_set || !(m_controlled_axis.mask ^ 2)) ? 2 : (m_controlled_axis.mask ^ 2);
            m_last_axis_set = GetTime();
            *m_sdf_creator.getSelected().value() = before_edit;
        }
        if (IsKeyPressed(KEY_Z))
        {
            m_controlled_axis.mask = (should_set || !(m_controlled_axis.mask ^ 4)) ? 4 : (m_controlled_axis.mask ^ 4);
            m_last_axis_set = GetTime();
            *m_sdf_creator.getSelected().value() = before_edit;
        }

        if (m_mouse_action == Control::CONTROL_TRANSLATE)
        {
            if (m_controlled_axis.x + m_controlled_axis.y + m_controlled_axis.z == 1)
            {
                Vector3 nearest = NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                                     Vector3Add(m_sdf_creator.getSelected().value()->pos,
                                                                (Vector3){static_cast<float>(m_controlled_axis.x),
                                                                          static_cast<float>(m_controlled_axis.y),
                                                                          static_cast<float>(m_controlled_axis.z)}),
                                                     ray.position, Vector3Add(ray.position, ray.direction));

                m_sdf_creator.getSelected().value()->pos = nearest;
            }
            else
            {
                Vector3 plane_normal;
                Vector3 intersection;
                if (m_controlled_axis.x + m_controlled_axis.y + m_controlled_axis.z == 2)
                {
                    plane_normal =
                        (Vector3){static_cast<float>(!m_controlled_axis.x), static_cast<float>(!m_controlled_axis.y),
                                  static_cast<float>(!m_controlled_axis.z)};
                }
                else
                {
                    plane_normal = Vector3Subtract(m_camera.position, m_camera.target);
                }

                if (RayPlaneIntersection(ray.position, ray.direction, m_sdf_creator.getSelected().value()->pos,
                                         plane_normal, &intersection))
                {
                    m_sdf_creator.getSelected().value()->pos = intersection;
                }
            }
        }
        else if (m_mouse_action == Control::CONTROL_ROTATE)
        {
            if (m_controlled_axis.x)
                m_sdf_creator.getSelected().value()->angle.x += GetMouseDelta().x / 10.f;
            if (m_controlled_axis.y)
                m_sdf_creator.getSelected().value()->angle.y += GetMouseDelta().x / 10.f;
            if (m_controlled_axis.z)
                m_sdf_creator.getSelected().value()->angle.z += GetMouseDelta().x / 10.f;
        }
        else if (m_mouse_action == Control::CONTROL_SCALE)
        {
            if (m_controlled_axis.x)
                m_sdf_creator.getSelected().value()->size.x *= powf(2, GetMouseDelta().x / 10.f);
            if (m_controlled_axis.y)
                m_sdf_creator.getSelected().value()->size.y *= powf(2, GetMouseDelta().x / 10.f);
            if (m_controlled_axis.z)
                m_sdf_creator.getSelected().value()->size.z *= powf(2, GetMouseDelta().x / 10.f);
        }

        if (IsKeyPressed(KEY_ESCAPE))
        {
            m_mouse_action = Control::CONTROL_NONE;
            m_sdf_creator.deselect();
            *m_sdf_creator.getSelected().value() = before_edit;
        }
        if (IsKeyPressed(KEY_ENTER))
            m_mouse_action = Control::CONTROL_NONE;
    }
    if (IsKeyPressed(KEY_ESCAPE) && m_sdf_creator.isSelected())
    {
        m_mouse_action = Control::CONTROL_NONE;
        m_sdf_creator.deselect();
    }
    static float drag_offset;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && m_mouse_action == Control::CONTROL_NONE &&
        m_sdf_creator.isSelected())
    {
        if (GetRayCollisionSphere(ray, Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0.6, 0, 0}), .1)
                .hit)
        {
            Vector3 nearest =
                NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                   Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){1, 0, 0}),
                                   ray.position, Vector3Add(ray.position, ray.direction));

            drag_offset = m_sdf_creator.getSelected().value()->pos.x - nearest.x;
            m_mouse_action = Control::CONTROL_POS_X;
        }
        else if (GetRayCollisionSphere(ray, Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 0.6, 0}),
                                       .1)
                     .hit)
        {
            Vector3 nearest =
                NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                   Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 1, 0}),
                                   ray.position, Vector3Add(ray.position, ray.direction));

            drag_offset = m_sdf_creator.getSelected().value()->pos.y - nearest.y;
            m_mouse_action = Control::CONTROL_POS_Y;
        }
        else if (GetRayCollisionSphere(ray, Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 0, 0.6}),
                                       .1)
                     .hit)
        {
            Vector3 nearest =
                NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                   Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 0, 1}),
                                   ray.position, Vector3Add(ray.position, ray.direction));

            drag_offset = m_sdf_creator.getSelected().value()->pos.z - nearest.z;
            m_mouse_action = Control::CONTROL_POS_Z;
        }
        else if (GetRayCollisionBox(
                     ray, boundingBoxSized(Vector3Add(m_sdf_creator.getSelected().value()->pos,
                                                      (Vector3){m_sdf_creator.getSelected().value()->size.x, 0, 0}),
                                           0.2))
                     .hit)
        {
            Vector3 nearest =
                NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                   Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){1, 0, 0}),
                                   ray.position, Vector3Add(ray.position, ray.direction));

            drag_offset = nearest.x - m_sdf_creator.getSelected().value()->size.x;
            m_mouse_action = Control::CONTROL_SCALE_X;
        }
        else if (GetRayCollisionBox(
                     ray, boundingBoxSized(Vector3Add(m_sdf_creator.getSelected().value()->pos,
                                                      (Vector3){0, m_sdf_creator.getSelected().value()->size.y, 0}),
                                           0.2))
                     .hit)
        {
            Vector3 nearest =
                NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                   Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 1, 0}),
                                   ray.position, Vector3Add(ray.position, ray.direction));

            drag_offset = nearest.y - m_sdf_creator.getSelected().value()->size.y;
            m_mouse_action = Control::CONTROL_SCALE_Y;
        }
        else if (GetRayCollisionBox(
                     ray, boundingBoxSized(Vector3Add(m_sdf_creator.getSelected().value()->pos,
                                                      (Vector3){0, 0, m_sdf_creator.getSelected().value()->size.z}),
                                           0.2))
                     .hit)
        {
            Vector3 nearest =
                NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                   Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 0, 1}),
                                   ray.position, Vector3Add(ray.position, ray.direction));

            drag_offset = nearest.z - m_sdf_creator.getSelected().value()->size.z;
            m_mouse_action = Control::CONTROL_SCALE_Z;
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && m_mouse_action == Control::CONTROL_NONE)
    {
        m_sdf_creator.objectAtPixel((int)GetMousePosition().x, (int)GetMousePosition().y, m_camera);
    }

    static Vector2 mouseDownPosition;
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        mouseDownPosition = GetMousePosition();

    if (!m_edit)
    {
        if (fabsf(GetMouseWheelMove()) > 0.01 && m_mouse_action == Control::CONTROL_NONE)
        {
            Vector2 delta = GetMouseWheelMoveV();

            if (IsKeyDown(KEY_LEFT_ALT))
            {
                CameraMoveForward(&m_camera, delta.y, false);
            }
            else
            {
                Vector3 shift = Vector3Scale(m_camera.up, delta.y / 10);
                m_camera.position = Vector3Add(m_camera.position, shift);
                m_camera.target = Vector3Add(m_camera.target, shift);
#ifdef PLATFORM_WEB
                delta.x = -delta.x;
#endif
                UpdateCameraPro(&m_camera, (Vector3){0, -delta.x / 10, 0}, Vector3Zero(), 0);
            }
        }
        else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && m_mouse_action == Control::CONTROL_NONE &&
                 Vector2Distance(mouseDownPosition, GetMousePosition()) > 1)
        {
            m_mouse_action = Control::CONTROL_ROTATE_CAMERA;
        }

        if (m_mouse_action == Control::CONTROL_ROTATE_CAMERA)
        {
            if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                m_mouse_action = Control::CONTROL_NONE;

            Vector2 delta = GetMouseDelta();
            if (IsKeyDown(KEY_LEFT_ALT))
            {
                UpdateCameraPro(&m_camera, (Vector3){0, -delta.x / 80, delta.y / 80}, Vector3Zero(), 0);
            }
            else
            {
                extern void CameraYaw(Camera * camera, float angle, bool rotateAroundTarget);
                extern void CameraPitch(Camera * camera, float angle, bool lockView, bool rotateAroundTarget,
                                        bool rotateUp);
                CameraYaw(&m_camera, -delta.x * 0.003f, true);
                CameraPitch(&m_camera, -delta.y * 0.003f, true, true, false);
            }
        }

#ifndef PLATFORM_WEB
        float magnification = 0;
        if (m_mouse_action == Control::CONTROL_NONE)
        {
            CameraMoveForward(&m_camera, 8 * magnification, false);
        }
        magnification = 0;
#endif
    }
    if (m_mouse_action == Control::CONTROL_POS_X)
    {
        Vector3 nearest = NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){1, 0, 0}),
                                             ray.position, Vector3Add(ray.position, ray.direction));

        m_sdf_creator.getSelected().value()->pos.x = nearest.x + drag_offset;
    }
    else if (m_mouse_action == Control::CONTROL_POS_Y)
    {
        Vector3 nearest = NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 1, 0}),
                                             ray.position, Vector3Add(ray.position, ray.direction));
        m_sdf_creator.getSelected().value()->pos.y = nearest.y + drag_offset;
    }
    else if (m_mouse_action == Control::CONTROL_POS_Z)
    {
        Vector3 nearest = NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 0, 1}),
                                             ray.position, Vector3Add(ray.position, ray.direction));
        m_sdf_creator.getSelected().value()->pos.z = nearest.z + drag_offset;
    }
    else if (m_mouse_action == Control::CONTROL_SCALE_X)
    {
        Vector3 nearest = NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){1, 0, 0}),
                                             ray.position, Vector3Add(ray.position, ray.direction));

        m_sdf_creator.getSelected().value()->size.x = fmaxf(0, nearest.x - drag_offset);
    }
    else if (m_mouse_action == Control::CONTROL_SCALE_Y)
    {
        Vector3 nearest = NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 1, 0}),
                                             ray.position, Vector3Add(ray.position, ray.direction));

        m_sdf_creator.getSelected().value()->size.y = fmaxf(0, nearest.y - drag_offset);
    }
    else if (m_mouse_action == Control::CONTROL_SCALE_Z)
    {
        Vector3 nearest = NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 0, 1}),
                                             ray.position, Vector3Add(ray.position, ray.direction));

        m_sdf_creator.getSelected().value()->size.z = fmaxf(0, nearest.z - drag_offset);
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        m_mouse_action = Control::CONTROL_NONE;
    }

    /*if (mode_mode_gamepad)
    {
        DrawCircle(sidebar_width + (GetScreenWidth() - sidebar_width) / 2, GetScreenHeight() / 2, 5, {255, 255, 255,
    255});
    }*/
}
void SDFCreatorGui::drawManipulator()
{
    BeginMode3D(m_camera);
    {
        if (m_sdf_creator.isSelected())
        {
            SDFObject s = *m_sdf_creator.getSelected().value();

            if (m_mouse_action == Control::CONTROL_TRANSLATE || m_mouse_action == Control::CONTROL_ROTATE ||
                m_mouse_action == Control::CONTROL_SCALE)
            {
                if (m_controlled_axis.x)
                    DrawRay((Ray){Vector3Add(s.pos, (Vector3){.x = -1000}), (Vector3){.x = 1}}, RED);
                if (m_controlled_axis.y)
                    DrawRay((Ray){Vector3Add(s.pos, (Vector3){.y = -1000}), (Vector3){.y = 1}}, GREEN);
                if (m_controlled_axis.z)
                    DrawRay((Ray){Vector3Add(s.pos, (Vector3){.z = -1000}), (Vector3){.z = 1}}, BLUE);
            }
            else
            {
                if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_X)
                    DrawLine3D(s.pos, Vector3Add(s.pos, (Vector3){0.5, 0, 0}), RED);
                if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_SCALE_X)
                    DrawCube(Vector3Add(s.pos, (Vector3){s.size.x, 0, 0}), .1, .1, .1, RED);
                if ((m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_X))
                    DrawCylinderEx(Vector3Add(s.pos, (Vector3){0.5, 0, 0}), Vector3Add(s.pos, (Vector3){.7, 0, 0}), .1,
                                   0, 12, RED);

                if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_Y)
                    DrawLine3D(s.pos, Vector3Add(s.pos, (Vector3){0, 0.5, 0}), GREEN);
                if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_SCALE_Y)
                    DrawCube(Vector3Add(s.pos, (Vector3){0, s.size.y, 0}), .1, .1, .1, GREEN);
                if ((m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_Y))
                    DrawCylinderEx(Vector3Add(s.pos, (Vector3){0, 0.5, 0}), Vector3Add(s.pos, (Vector3){0, .7, 0}), .1,
                                   0, 12, GREEN);

                if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_Z)
                    DrawLine3D(s.pos, Vector3Add(s.pos, (Vector3){0, 0, 0.5}), BLUE);
                if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_SCALE_Z)
                    DrawCube(Vector3Add(s.pos, (Vector3){0, 0, s.size.z}), .1, .1, .1, BLUE);
                if ((m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_Z))
                    DrawCylinderEx(Vector3Add(s.pos, (Vector3){0, 0, 0.5}), Vector3Add(s.pos, (Vector3){0, 0, 0.7}), .1,
                                   0, 12, BLUE);
            }
        }
    }
    EndMode3D();
}