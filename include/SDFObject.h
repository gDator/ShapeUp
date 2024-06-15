#ifndef SDFOBJECT_H
#define SDFOBJECT_H
#include "raylib.h"
#include <cstdint>
#include <sstream>
#include "SDFTypes.h"

class SDFObject final
{
public:
    explicit SDFObject(const SDFObjectType type) :
        m_object_type(type),
        m_type(SDFType::OBJECT)
    {}
    explicit SDFObject(const SDFOperationType type) :
        m_operation_type(type),
        m_type(SDFType::OPERATION)
    {}
    SDFObject() = default;
    [[nodiscard]] inline int getIndex() const { return m_index; }
    [[nodiscard]] inline SDFType getType() const { return m_type; }
    [[nodiscard]] inline SDFObjectType getObjectType() const { return m_object_type; }
    [[nodiscard]] inline SDFOperationType getOperationType() const { return m_operation_type; }
    std::string ToString() const;
    inline void reset() { m_done = false; }
    inline void done() { m_done = true; }
    [[nodiscard]] inline bool isDone() const { return m_done; }
    virtual ~SDFObject() = default;
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
    bool subtract = false;
    int m_index = 0;
private:
    SDFOperationType m_operation_type = SDFOperationType::NONE;
    SDFObjectType m_object_type = SDFObjectType::NONE;
    SDFType m_type = SDFType::OBJECT;
    bool m_done = false;
};
#endif // SDFOBJECT_H
