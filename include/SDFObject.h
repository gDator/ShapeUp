#ifndef SDFOBJECT_H
#define SDFOBJECT_H
#include "raylib.h"
#include <cstdint>
#include <sstream>
#include "SDFTypes.h"

struct SDFObject
{
    Vector3 pos{};
    Vector3 size{};
    Vector3 angle{};
    Vector3 flex_vector{1, 1, 1};
    float corner_radius{};
    float flex_parameter{};
    struct
    {
        uint8_t r, g, b;
    } color{};
    struct
    {
        bool x, y, z;
    } mirror{};
    int index = 0;
    SDFOperationType operation_type = SDFOperationType::NONE;
    SDFObjectType object_type = SDFObjectType::NONE;
    SDFType type = SDFType::OBJECT;
    bool done = false;

};

inline std::ostream& operator<<(std::ostream& stream, const SDFObject& data)
{
    stream << data.pos.x;
    stream << data.pos.y;
    stream << data.pos.z;

    stream << data.size.x;
    stream << data.size.y;
    stream << data.size.z;

    stream << data.angle.x;
    stream << data.angle.y;
    stream << data.angle.z;

    stream << data.flex_vector.x;
    stream << data.flex_vector.y;
    stream << data.flex_vector.z;
    stream << data.corner_radius;

    stream << data.flex_parameter;

    stream << data.color.r;
    stream << data.color.g;
    stream << data.color.b;

    stream << data.mirror.x;
    stream << data.mirror.y;
    stream << data.mirror.z;
    stream << data.index;
    stream << static_cast<int>(data.operation_type);
    stream << static_cast<int>(data.object_type);
    stream << static_cast<int>(data.type);
    stream << data.done;
    return stream;
}

inline std::istream& operator>>(std::istream& stream, SDFObject& data)
{
    int op_type = 0, obj_type = 0, type = 0;
    stream >> data.pos.x;
    stream >> data.pos.y;
    stream >> data.pos.z;

    stream >> data.size.x;
    stream >> data.size.y;
    stream >> data.size.z;

    stream >> data.angle.x;
    stream >> data.angle.y;
    stream >> data.angle.z;

    stream >> data.flex_vector.x;
    stream >> data.flex_vector.y;
    stream >> data.flex_vector.z;
    stream >> data.corner_radius;

    stream >> data.flex_parameter;

    stream >> data.color.r;
    stream >> data.color.g;
    stream >> data.color.b;

    stream >> data.mirror.x;
    stream >> data.mirror.y;
    stream >> data.mirror.z;
    stream >> data.index;

    stream >> op_type;
    stream >> obj_type;
    stream >> type;
    stream >> data.done;

    data.operation_type = static_cast<SDFOperationType>(op_type);
    data.object_type = static_cast<SDFObjectType>(obj_type);
    data.type = static_cast<SDFType>(type);

    return stream;
}

static std::string ObjectToString(const SDFObject& s)
{
    if (s.type == SDFType::OBJECT)
    {
        return (object_to_string.at(s.object_type) + std::to_string(s.index));
    }
    return (operation_to_string.at(s.operation_type) + std::to_string(s.index));
}
#endif // SDFOBJECT_H
