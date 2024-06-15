#include "SDFObject.h"

std::string SDFObject::ToString() const
{
    if (m_type == SDFType::OBJECT)
    {
        return (object_to_string.at(m_object_type) + std::to_string(m_index));
    }
    return (operation_to_string.at(m_operation_type) + std::to_string(m_index));
}