//
// Created by Daniel Hagen on 10.06.2024.
//

#ifndef SDFCREATORGUI_H
#define SDFCREATORGUI_H
#include "SDFCreator.h"
#include "raylib.h"

enum class Control
{
    CONTROL_NONE,
    CONTROL_POS_X,
    CONTROL_POS_Y,
    CONTROL_POS_Z,
    CONTROL_SCALE_X,
    CONTROL_SCALE_Y,
    CONTROL_SCALE_Z,
    CONTROL_ANGLE_X,
    CONTROL_ANGLE_Y,
    CONTROL_ANGLE_Z,
    CONTROL_COLOR_R,
    CONTROL_COLOR_G,
    CONTROL_COLOR_B,
    CONTROL_TRANSLATE,
    CONTROL_ROTATE,
    CONTROL_SCALE,
    CONTROL_CORNER_RADIUS,
    CONTROL_ROTATE_CAMERA,
    CONTROL_BLOB_AMOUNT,
};

class SDFCreatorGui
{
public:
    SDFCreatorGui();
    void run();

private:
    Camera2D m_texture_camera{};
    Camera m_camera {};
    RenderTexture m_texture{};
    SDFCreator m_sdf_creator{};
    int m_screen_width = 1200;
    int m_screen_height = 800;
    Control m_focused_control{};
    Control m_mouse_action{};
    double m_last_axis_set = 0;
    bool mode_mode_gamepad = false;
    union
    {
        struct
        {
            bool x : 1;
            bool y : 1;
            bool z : 1;
        };
        int mask;
    } m_controlled_axis = {.mask = 0x7};
private:
    void drawGui();
    void handleInput();
};

#endif //SDFCREATORGUI_H
