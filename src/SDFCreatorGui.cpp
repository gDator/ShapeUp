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

SDFCreatorGui::SDFCreatorGui()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(m_screen_width, m_screen_height, "Sudoku");
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
        if (m_sdf_creator.loadShader(m_camera))
        {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), DARKBLUE);
            m_sdf_creator.unloadShader();
        }
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

    static float last_color[3] = {1,1,1};
    if (ImGui::Begin("Edit"))
    {
        if (ImGui::Button("Save"))
        {
            m_sdf_creator.save("./");
        }
        ImGui::SameLine();
        if (ImGui::Button("Export"))
        {
            m_sdf_creator.exportObj();
        }
        if (m_sdf_creator.isSelected())
        {
            SDFObject* old = m_sdf_creator.getSelected().value();
            ImGui::InputFloat3("Position", &old->pos.x);
            ImGui::InputFloat3("Scale", &old->size.x);
            ImGui::InputFloat3("Rotation", &old->angle.x);
            if(ImGui::ColorPicker3("Color", last_color))
            {
                old->color.r = last_color[0] * 255;
                old->color.g = last_color[1] * 255;
                old->color.b = last_color[2] * 255;
            }
            ImGui::SliderFloat("Blob", &old->blob_amount, 0, 10);
            ImGui::SliderFloat("Roundness", &old->corner_radius, 0, 9999);
            ImGui::Checkbox("Cut out", &old->subtract);
            ImGui::Text("Mirror");
            ImGui::Checkbox("X", &old->mirror.x);
            ImGui::SameLine();
            ImGui::Checkbox("Y", &old->mirror.y);
            ImGui::SameLine();
            ImGui::Checkbox("Z", &old->mirror.z);

            if (memcmp(&old->mirror, &m_sdf_creator.getSelected().value()->mirror, sizeof(old->mirror)) ||
                old->subtract != m_sdf_creator.getSelected().value()->subtract)
            {
                m_sdf_creator.rebuild();

                BoundingBox bb = m_sdf_creator.shapeBoundingBox(*m_sdf_creator.getSelected().value());
                if (m_sdf_creator.getSelected().value()->mirror.x && bb.max.x <= 0)
                {
                    m_sdf_creator.getSelected().value()->pos.x *= -1;
                    m_sdf_creator.getSelected().value()->angle.y *= -1;
                    m_sdf_creator.getSelected().value()->angle.z *= -1;
                }

                if (m_sdf_creator.getSelected().value()->mirror.y && bb.max.y <= 0)
                {
                    m_sdf_creator.getSelected().value()->pos.y *= -1;
                    m_sdf_creator.getSelected().value()->angle.x *= -1;
                    m_sdf_creator.getSelected().value()->angle.z *= -1;
                }

                if (m_sdf_creator.getSelected().value()->mirror.z && bb.max.z <= 0)
                {
                    m_sdf_creator.getSelected().value()->pos.z *= -1;
                    m_sdf_creator.getSelected().value()->angle.y *= -1;
                    m_sdf_creator.getSelected().value()->angle.x *= -1;
                }
            }
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
            m_sdf_creator.deleteSphere();
        }

        if (m_sdf_creator.isSelected() && IsKeyPressed(KEY_D) &&
            (IsKeyDown(KEY_RIGHT_SUPER) || IsKeyDown(KEY_LEFT_SUPER)))
        {
            // TODO: ??
            //  spheres[num_spheres] = m_sdf_creator.getSelected();
            //  selected_sphere = num_spheres;
            //  num_spheres++;
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
                Vector3 nearest =
                    SDFCreator::NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
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

                if (m_sdf_creator.RayPlaneIntersection(ray.position, ray.direction,
                                                       m_sdf_creator.getSelected().value()->pos, plane_normal,
                                                       &intersection))
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
            *m_sdf_creator.getSelected().value() = before_edit;
        }
        if (IsKeyPressed(KEY_ENTER))
            m_mouse_action = Control::CONTROL_NONE;
    }

    static float drag_offset;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && m_mouse_action == Control::CONTROL_NONE &&
        m_sdf_creator.isSelected())
    {
        if (GetRayCollisionSphere(ray, Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0.6, 0, 0}), .1)
                .hit)
        {
            Vector3 nearest = m_sdf_creator.NearestPointOnLine(
                m_sdf_creator.getSelected().value()->pos,
                Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){1, 0, 0}), ray.position,
                Vector3Add(ray.position, ray.direction));

            drag_offset = m_sdf_creator.getSelected().value()->pos.x - nearest.x;
            m_mouse_action = Control::CONTROL_POS_X;
        }
        else if (GetRayCollisionSphere(ray, Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 0.6, 0}),
                                       .1)
                     .hit)
        {
            Vector3 nearest = m_sdf_creator.NearestPointOnLine(
                m_sdf_creator.getSelected().value()->pos,
                Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 1, 0}), ray.position,
                Vector3Add(ray.position, ray.direction));

            drag_offset = m_sdf_creator.getSelected().value()->pos.y - nearest.y;
            m_mouse_action = Control::CONTROL_POS_Y;
        }
        else if (GetRayCollisionSphere(ray, Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 0, 0.6}),
                                       .1)
                     .hit)
        {
            Vector3 nearest = m_sdf_creator.NearestPointOnLine(
                m_sdf_creator.getSelected().value()->pos,
                Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 0, 1}), ray.position,
                Vector3Add(ray.position, ray.direction));

            drag_offset = m_sdf_creator.getSelected().value()->pos.z - nearest.z;
            m_mouse_action = Control::CONTROL_POS_Z;
        }
        else if (GetRayCollisionBox(ray, m_sdf_creator.boundingBoxSized(
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos,
                                                        (Vector3){m_sdf_creator.getSelected().value()->size.x, 0, 0}),
                                             0.2))
                     .hit)
        {
            Vector3 nearest = m_sdf_creator.NearestPointOnLine(
                m_sdf_creator.getSelected().value()->pos,
                Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){1, 0, 0}), ray.position,
                Vector3Add(ray.position, ray.direction));

            drag_offset = nearest.x - m_sdf_creator.getSelected().value()->size.x;
            m_mouse_action = Control::CONTROL_SCALE_X;
        }
        else if (GetRayCollisionBox(ray, m_sdf_creator.boundingBoxSized(
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos,
                                                        (Vector3){0, m_sdf_creator.getSelected().value()->size.y, 0}),
                                             0.2))
                     .hit)
        {
            Vector3 nearest = m_sdf_creator.NearestPointOnLine(
                m_sdf_creator.getSelected().value()->pos,
                Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 1, 0}), ray.position,
                Vector3Add(ray.position, ray.direction));

            drag_offset = nearest.y - m_sdf_creator.getSelected().value()->size.y;
            m_mouse_action = Control::CONTROL_SCALE_Y;
        }
        else if (GetRayCollisionBox(ray, m_sdf_creator.boundingBoxSized(
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos,
                                                        (Vector3){0, 0, m_sdf_creator.getSelected().value()->size.z}),
                                             0.2))
                     .hit)
        {
            Vector3 nearest = m_sdf_creator.NearestPointOnLine(
                m_sdf_creator.getSelected().value()->pos,
                Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 0, 1}), ray.position,
                Vector3Add(ray.position, ray.direction));

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

    const float movement_scale = 0.1;
    const float rotation_scale = 0.04;
    const float rotation_radius = 4;

    /*if (!IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_THUMB))
    {
        Vector3 offset = Vector3Scale(GetCameraForward(&m_camera), -rotation_radius);
        offset = Vector3RotateByAxisAngle(offset, (Vector3){0, 1, 0},
                                          -rotation_scale * GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X));
        offset = Vector3RotateByAxisAngle(offset, GetCameraRight(&m_camera),
                                          -rotation_scale * GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y));
        m_camera.position = Vector3Add(offset, m_camera.target);
    }

    if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_1))
    {
        Vector3 up = Vector3Normalize(Vector3CrossProduct(GetCameraForward(&m_camera), GetCameraRight(&m_camera)));
        up = Vector3Scale(up, movement_scale * GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y));
        m_camera.position = Vector3Add(m_camera.position, up);
        m_camera.target = Vector3Add(m_camera.target, up);
    }
    else
    {
        CameraMoveForward(&camera, -GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y) * movement_scale, false);
    }

    CameraMoveRight(&camera, GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X) * movement_scale, false);

    Matrix camera_matrix = GetCameraMatrix(camera);
    static Vector3 camera_space_offset;
    if (IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
    {
        selected_sphere =
            object_at_pixel(sidebar_width + (GetScreenWidth() - sidebar_width) / 2, GetScreenHeight() / 2);
        needs_rebuild = true;
        if (m_sdf_creator.isSelected())
        {
            camera_space_offset = WorldToCamera(m_sdf_creator.getSelected().value()->pos, camera_matrix);
        }
    }

    if (m_sdf_creator.isSelected())
    {
        if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
        {
            m_sdf_creator.getSelected().value()->pos = CameraToWorld(camera_space_offset, camera_matrix);
            // m_sdf_creator.getSelected().value()->pos = Vector3Add(camera.position,
            // Vector3Scale(GetCameraForward(&camera),distance));
        }

        if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_UP))
        {
            m_sdf_creator.getSelected().value()->size = Vector3Scale(m_sdf_creator.getSelected().value()->size, 1.05);
            m_sdf_creator.getSelected().value()->corner_radius *= 1.05;
        }
        if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
        {
            m_sdf_creator.getSelected().value()->size = Vector3Scale(m_sdf_creator.getSelected().value()->size, 0.95);
            m_sdf_creator.getSelected().value()->corner_radius *= 0.95;
        }

        if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_1))
        {
            if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_LEFT))
            {
                m_sdf_creator.getSelected().value()->blob_amount *= 0.95;
            }
            if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
            {
                m_sdf_creator.getSelected().value()->blob_amount = (0.01 +
    m_sdf_creator.getSelected().value()->blob_amount * 1.05);
            }
        }
        else
        {
            if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_LEFT))
            {
                m_sdf_creator.getSelected().value()->corner_radius *= 0.95;
            }
            if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
            {
                Vector3 size = m_sdf_creator.getSelected().value()->size;
                m_sdf_creator.getSelected().value()->corner_radius = fminf(
                    0.01 + m_sdf_creator.getSelected().value()->corner_radius * 1.05, fminf(size.x, fminf(size.y,
    size.z)));
            }
        }

        if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_THUMB))
        {
            m_sdf_creator.getSelected().value()->angle =
                Vector3Add(m_sdf_creator.getSelected().value()->angle,
                           (Vector3){
                               rotation_scale * GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y),
                               rotation_scale * GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X),
                               0,
                           });
        }
    }

    if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
    {
        if (IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
        {
            add_shape();
        }

        if (m_sdf_creator.isSelected())
            m_sdf_creator.getSelected().value()->pos = Vector3Add(camera.position,
    Vector3Scale(GetCameraForward(&camera), 8));
    }*/

    if (m_mouse_action == Control::CONTROL_POS_X)
    {
        Vector3 nearest =
            m_sdf_creator.NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){1, 0, 0}),
                                             ray.position, Vector3Add(ray.position, ray.direction));

        m_sdf_creator.getSelected().value()->pos.x = nearest.x + drag_offset;
    }
    else if (m_mouse_action == Control::CONTROL_POS_Y)
    {
        Vector3 nearest =
            m_sdf_creator.NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 1, 0}),
                                             ray.position, Vector3Add(ray.position, ray.direction));
        m_sdf_creator.getSelected().value()->pos.y = nearest.y + drag_offset;
    }
    else if (m_mouse_action == Control::CONTROL_POS_Z)
    {
        Vector3 nearest =
            m_sdf_creator.NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 0, 1}),
                                             ray.position, Vector3Add(ray.position, ray.direction));
        m_sdf_creator.getSelected().value()->pos.z = nearest.z + drag_offset;
    }
    else if (m_mouse_action == Control::CONTROL_SCALE_X)
    {
        Vector3 nearest =
            m_sdf_creator.NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){1, 0, 0}),
                                             ray.position, Vector3Add(ray.position, ray.direction));

        m_sdf_creator.getSelected().value()->size.x = fmaxf(0, nearest.x - drag_offset);
    }
    else if (m_mouse_action == Control::CONTROL_SCALE_Y)
    {
        Vector3 nearest =
            m_sdf_creator.NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 1, 0}),
                                             ray.position, Vector3Add(ray.position, ray.direction));

        m_sdf_creator.getSelected().value()->size.y = fmaxf(0, nearest.y - drag_offset);
    }
    else if (m_mouse_action == Control::CONTROL_SCALE_Z)
    {
        Vector3 nearest =
            m_sdf_creator.NearestPointOnLine(m_sdf_creator.getSelected().value()->pos,
                                             Vector3Add(m_sdf_creator.getSelected().value()->pos, (Vector3){0, 0, 1}),
                                             ray.position, Vector3Add(ray.position, ray.direction));

        m_sdf_creator.getSelected().value()->size.z = fmaxf(0, nearest.z - drag_offset);
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        m_mouse_action = Control::CONTROL_NONE;
    }

    if (m_sdf_creator.isSelected()) //>= 0 && selected_sphere < MAX_SPHERES
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
            if ((m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_X) &&
                !mode_mode_gamepad)
                DrawCylinderEx(Vector3Add(s.pos, (Vector3){0.5, 0, 0}), Vector3Add(s.pos, (Vector3){.7, 0, 0}), .1, 0,
                               12, RED);

            if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_Y)
                DrawLine3D(s.pos, Vector3Add(s.pos, (Vector3){0, 0.5, 0}), GREEN);
            if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_SCALE_Y)
                DrawCube(Vector3Add(s.pos, (Vector3){0, s.size.y, 0}), .1, .1, .1, GREEN);
            if ((m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_Y) &&
                !mode_mode_gamepad)
                DrawCylinderEx(Vector3Add(s.pos, (Vector3){0, 0.5, 0}), Vector3Add(s.pos, (Vector3){0, .7, 0}), .1, 0,
                               12, GREEN);

            if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_Z)
                DrawLine3D(s.pos, Vector3Add(s.pos, (Vector3){0, 0, 0.5}), BLUE);
            if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_SCALE_Z)
                DrawCube(Vector3Add(s.pos, (Vector3){0, 0, s.size.z}), .1, .1, .1, BLUE);
            if ((m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_Z) &&
                !mode_mode_gamepad)
                DrawCylinderEx(Vector3Add(s.pos, (Vector3){0, 0, 0.5}), Vector3Add(s.pos, (Vector3){0, 0, 0.7}), .1, 0,
                               12, BLUE);
        }
    }
    BeginMode3D(m_camera); {
                if (m_sdf_creator.isSelected()) {
                    SDFObject s = *m_sdf_creator.getSelected().value();

                    if (m_mouse_action == Control::CONTROL_TRANSLATE || m_mouse_action == Control::CONTROL_ROTATE || m_mouse_action == Control::CONTROL_SCALE) {
                        if (m_controlled_axis.x) DrawRay((Ray){Vector3Add(s.pos, (Vector3){.x=-1000}), (Vector3){.x=1}} , RED);
                        if (m_controlled_axis.y) DrawRay((Ray){Vector3Add(s.pos, (Vector3){.y=-1000}), (Vector3){.y=1}} , GREEN);
                        if (m_controlled_axis.z) DrawRay((Ray){Vector3Add(s.pos, (Vector3){.z=-1000}), (Vector3){.z=1}} , BLUE);
                    } else {

                        if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_X)
                            DrawLine3D(s.pos, Vector3Add(s.pos, (Vector3){0.5,0,0}),  RED);
                        if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_SCALE_X)
                            DrawCube(Vector3Add(s.pos, (Vector3){s.size.x,0,0}), .1,.1,.1, RED);
                        if ((m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_X))
                            DrawCylinderEx(Vector3Add(s.pos, (Vector3){0.5,0,0}),
                                           Vector3Add(s.pos, (Vector3){.7,0,0}), .1, 0, 12, RED);

                        if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_Y)
                            DrawLine3D(s.pos, Vector3Add(s.pos, (Vector3){0,0.5,0}),  GREEN);
                        if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_SCALE_Y)
                            DrawCube(Vector3Add(s.pos, (Vector3){0,s.size.y,0}), .1,.1,.1, GREEN);
                        if ((m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_Y))
                            DrawCylinderEx(Vector3Add(s.pos, (Vector3){0,0.5,0}),
                                           Vector3Add(s.pos, (Vector3){0,.7,0}), .1, 0, 12, GREEN);

                        if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_Z)
                            DrawLine3D(s.pos, Vector3Add(s.pos, (Vector3){0,0,0.5}),  BLUE);
                        if (m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_SCALE_Z)
                            DrawCube(Vector3Add(s.pos, (Vector3){0,0,s.size.z}), .1,.1,.1, BLUE);
                        if ((m_mouse_action == Control::CONTROL_NONE || m_mouse_action == Control::CONTROL_POS_Z))
                            DrawCylinderEx(Vector3Add(s.pos, (Vector3){0,0,0.5}),
                                           Vector3Add(s.pos, (Vector3){0,0,0.7}), .1, 0, 12, BLUE);

                    }
                }
            } EndMode3D();

    /*if (mode_mode_gamepad)
    {
        DrawCircle(sidebar_width + (GetScreenWidth() - sidebar_width) / 2, GetScreenHeight() / 2, 5, {255, 255, 255,
    255});
    }*/
}