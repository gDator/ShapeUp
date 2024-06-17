#ifndef SDFTYPES_H
#define SDFTYPES_H

#include <unordered_map>
#include <vector>
enum class SDFObjectType : int
{
    NONE,
    ROUND_BOX,
    SPHERE,
    BOX_FRAME,
    TORUS,
    BOX,
    CAPPED_TORUS,
    LINK,
    CYLINDER,
    CONE,
    PLANE,
    HEX_PRISM,
    TRI_PRISM,
    CAPSULE,
    VERTICAL_CAPSULE,
    CAPPED_CYLINDER,
    ROUNDED_CYLINDER,
    CAPPED_CONE,
    SOLDID_ANGLE,
    CUT_SPHERE,
    CUT_HOLLOW_SPHERE,
    DEATH_STAR,
    ROUND_CONE,
    // ROUNDED_CONE,
    ELLIPSOID,
    VESICA_SEGMENT,
    RHOMBUS,
    OCTAHEDRON,
    PYRAMID,
    // TRIANGLE,
    // QUAD
};

enum class SDFOperationType : int
{
    NONE,
    SMOOTH_UNION,
    SMOOTH_SUBSTRACTION,
    SMOOTH_INTERSECTION,
    ROUND,
    ONION,
    UNION,
    SUBSTRACTION,
    INTERSECTION,
};
enum class SDFType : int
{
    OBJECT,
    OPERATION
};
inline const std::unordered_map<SDFType, std::string> types_to_string = {
    {SDFType::OBJECT, "Object"},
    {SDFType::OPERATION, "OPERATION"}
};
inline const std::unordered_map<SDFObjectType, std::string> object_to_string = {
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

inline static const std::unordered_map<SDFOperationType, std::string> operation_to_string = {
    {SDFOperationType::NONE, "NONE"},
    {SDFOperationType::SMOOTH_UNION, "SMOOTH UNION"},
    {SDFOperationType::SMOOTH_SUBSTRACTION, "SMOOTH_SUBSTRACTION"},
    {SDFOperationType::SMOOTH_INTERSECTION, "SMOOTH_INTERSECTION"},
    {SDFOperationType::ROUND, "ROUND"},
    {SDFOperationType::ONION, "ONION"},
    {SDFOperationType::UNION, "UNION"},
    {SDFOperationType::SUBSTRACTION, "SUBSTRACTION"},
    {SDFOperationType::INTERSECTION, "INTERSECTION"}};
#endif // SDFTYPES_H
