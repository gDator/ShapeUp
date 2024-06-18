//
// Created by Daniel Hagen on 18.06.2024.
//

#ifndef VERTEX_H
#define VERTEX_H

struct Vertex
{
    struct
    {
        float x, y, z;
    }vertex;
    struct
    {
        unsigned char r, g, b;
    }color;
};

#endif //VERTEX_H
