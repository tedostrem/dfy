#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <sys/types.h>
#include <libgte.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

struct mesh {
    SVECTOR *vertices;
    short *faces;
    SVECTOR *normals;
    size_t vertex_count;
    size_t face_count;
    size_t normal_count;
};

struct object {
    SVECTOR rotation;
    VECTOR position;
    VECTOR scale;
    VECTOR vel;
    VECTOR acc;
    struct mesh *mesh;
    CVECTOR color;
};

struct object* create_cube(void);
struct object* create_floor(void);

#endif