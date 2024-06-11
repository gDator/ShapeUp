//
// Created by Daniel Hagen on 10.06.2024.
//

#ifndef SDFOBJECT_H
#define SDFOBJECT_H
#include "raylib.h"
#include <cstdint>

class SDFCreator;
class SDFObject
{
    friend SDFCreator;
public:
    Vector3 pos {};
    Vector3 size {};
    Vector3 angle {};
    float corner_radius {};
    float blob_amount {};
    struct
    {
        uint8_t r, g, b;
    } color {};
    struct
    {
        bool x, y, z;
    } mirror {};
    bool subtract = false;
};
#endif //SDFOBJECT_H
