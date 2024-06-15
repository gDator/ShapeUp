//
// Created by Daniel Hagen on 10.06.2024.
//

#ifndef SDFCREATORGUI_H
#define SDFCREATORGUI_H
#include "SDFCreator.h"
#include "raylib.h"
#include "SDFTypes.h"

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
    Camera m_camera{};
    RenderTexture m_texture{};
    SDFCreator m_sdf_creator{};
    int m_screen_width = 1200;
    int m_screen_height = 800;
    Control m_focused_control{};
    Control m_mouse_action{};
    double m_last_axis_set = 0;
    bool mode_mode_gamepad = false;
    bool m_show_sdf_field = false;
    bool m_edit = true;
    float m_blend = 0.5;
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
    void drawManipulator();

    SDFType m_type = SDFType::OBJECT;
    SDFObjectType m_selected_object_type = SDFObjectType::NONE;
    SDFOperationType m_selected_operation_type = SDFOperationType::NONE;
    SDFIterator m_selected_iterator;

    inline static const std::vector<std::tuple<SDFOperationType, std::string>> operation_types = {
        {SDFOperationType::NONE, "NONE"},
        {SDFOperationType::SMOOTH_UNION, "SMOOTH UNION"},
        {SDFOperationType::SMOOTH_SUBSTRACTION, "SMOOTH_SUBSTRACTION"},
        {SDFOperationType::SMOOTH_INTERSECTION, "SMOOTH_INTERSECTION"},
        {SDFOperationType::ROUND, "ROUND"},
        {SDFOperationType::ONION, "ONION"},
        {SDFOperationType::UNION, "UNION"},
        {SDFOperationType::SUBSTRACTION, "SUBSTRACTION"},
        {SDFOperationType::INTERSECTION, "INTERSECTION"}};
    inline static const std::vector<std::tuple<SDFObjectType, std::string>> object_types = {
        {SDFObjectType::NONE, "NONE"},
        {SDFObjectType::ROUND_BOX, "ROUND_BOX"},
        {SDFObjectType::SPHERE, "SPHERE"},
        {SDFObjectType::BOX_FRAME, "BOX_FRAME"},
        {SDFObjectType::TORUS, "TORUS"},
        {SDFObjectType::BOX, "BOX"},
        {SDFObjectType::CAPPED_TORUS, "CAPPED_TORUS"},
        {SDFObjectType::LINK, "LINK"},
        {SDFObjectType::CYLINDER, "CYLINDER"},
        {SDFObjectType::CONE, "CONE"},
        {SDFObjectType::PLANE, "PLANE"},
        {SDFObjectType::HEX_PRISM, "HEX_PRISM"},
        {SDFObjectType::TRI_PRISM, "TRI_PRISM"},
        {SDFObjectType::CAPSULE, "CAPSULE"},
        {SDFObjectType::VERTICAL_CAPSULE, "VERTICAL_CAPSULE"},
        {SDFObjectType::CAPPED_CYLINDER, "CAPPED_CYLINDER"},
        {SDFObjectType::ROUNDED_CYLINDER, "ROUNDED_CYLINDER"},
        {SDFObjectType::CAPPED_CONE, "CAPPED_CONE"},
        {SDFObjectType::SOLDID_ANGLE, "SOLDID_ANGLE"},
        {SDFObjectType::CUT_SPHERE, "CUT_SPHERE"},
        {SDFObjectType::CUT_HOLLOW_SPHERE, "CUT_HOLLOW_SPHERE"},
        {SDFObjectType::DEATH_STAR, "DEATH_STAR"},
        {SDFObjectType::ROUND_CONE, "ROUND_CONE"},
        // {SDFObjectType::ROUNDED_CONE, "ROUNDED_CONE"},
        {SDFObjectType::ELLIPSOID, "ELLIPSOID"},
        {SDFObjectType::VESICA_SEGMENT, "VESICA_SEGMENT"},
        {SDFObjectType::RHOMBUS, "RHOMBUS"},
        {SDFObjectType::OCTAHEDRON, "OCTAHEDRON"},
        {SDFObjectType::PYRAMID, "PYRAMID"},
        // {SDFObjectType::TRIANGLE, "TRIANGLE"},
        // {SDFObjectType::QUAD, "QUAD"}
    };
};

#endif // SDFCREATORGUI_H
