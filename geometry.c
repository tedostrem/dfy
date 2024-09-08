#include <stdlib.h>
#include <string.h>
#include "geometry.h"

#define CUBESIZE 196

SVECTOR cube_vertices[] = {
    {-CUBESIZE / 2, -CUBESIZE / 2, -CUBESIZE / 2, 0}, // Vertex 0
    {CUBESIZE / 2, -CUBESIZE / 2, -CUBESIZE / 2, 0},  // Vertex 1
    {CUBESIZE / 2, CUBESIZE / 2, -CUBESIZE / 2, 0},   // Vertex 2
    {-CUBESIZE / 2, CUBESIZE / 2, -CUBESIZE / 2, 0},  // Vertex 3
    {-CUBESIZE / 2, -CUBESIZE / 2, CUBESIZE / 2, 0},  // Vertex 4
    {CUBESIZE / 2, -CUBESIZE / 2, CUBESIZE / 2, 0},   // Vertex 5
    {CUBESIZE / 2, CUBESIZE / 2, CUBESIZE / 2, 0},    // Vertex 6
    {-CUBESIZE / 2, CUBESIZE / 2, CUBESIZE / 2, 0}    // Vertex 7
};

short cube_faces[] = {
    0, 1, 2,  0, 2, 3,  // Front face
    1, 5, 6,  1, 6, 2,  // Right face
    5, 4, 7,  5, 7, 6,  // Back face
    4, 0, 3,  4, 3, 7,  // Left face
    4, 5, 1,  4, 1, 0,  // Bottom face
    3, 2, 6,  3, 6, 7   // Top face
};

SVECTOR cube_normals[12]; // Normals for the cube faces

SVECTOR floor_vertices[] = {
    {-900, 0, -900}, {-300, 0, -900}, {300, 0, -900}, {900, 0, -900},  // First row
    {-900, 0, -300}, {-300, 0, -300}, {300, 0, -300}, {900, 0, -300},  // Second row
    {-900, 0,  300}, {-300, 0,  300}, {300, 0,  300}, {900, 0,  300},  // Third row
    {-900, 0,  900}, {-300, 0,  900}, {300, 0,  900}, {900, 0,  900},  // Fourth row
};

short floor_faces[] = {
    // First quad
    0, 4, 1,
    1, 4, 5,
    // Second quad
    1, 5, 2,
    2, 5, 6,
    // Third quad
    2, 6, 3,
    3, 6, 7,
    // Fourth quad
    4, 8, 5,
    5, 8, 9,
    // Fifth quad
    5, 9, 6,
    6, 9, 10,
    // Sixth quad
    6, 10, 7,
    7, 10, 11,
    // Seventh quad
    8, 12, 9,
    9, 12, 13,
    // Eighth quad
    9, 13, 10,
    10, 13, 14,
    // Ninth quad
    10, 14, 11,
    11, 14, 15,
};

SVECTOR floor_normals[16];  // For 16 triangles



struct mesh cube_mesh = {
    cube_vertices,
    cube_faces,
    cube_normals,
    ARRAY_SIZE(cube_vertices),
    ARRAY_SIZE(cube_faces),
    ARRAY_SIZE(cube_normals)
};

struct mesh floor_mesh = {
    floor_vertices,
    floor_faces,
    floor_normals,
    ARRAY_SIZE(floor_vertices),
    ARRAY_SIZE(floor_faces),
    ARRAY_SIZE(floor_normals),
};

CVECTOR floor_color = {200, 200, 255};
CVECTOR cube_color = {255, 200, 255};

struct object object_cube = {
    {0, 0, 0},
    {0, -400, 1800},
    {ONE, ONE, ONE},
    {0, 0, 0},
    {0, 1, 0},
    &cube_mesh,
    {255, 200, 255}
};

struct object object_floor = {
    {0, 0, 0},
    {0, 450, 1800},
    {ONE, ONE, ONE},
    {0, 0, 0},
    {0, 0, 0},
    &floor_mesh,
    {255, 255, 255}
};

struct object* create_cube(void) {
    struct object *o = (struct object*)malloc3(sizeof(struct object));
    memcpy(o, &object_cube, sizeof(struct object));
    return o;
}

struct object* create_floor(void) {
    struct object *o = (struct object*)malloc3(sizeof(struct object));
    memcpy(o, &object_floor, sizeof(struct object));
    return o;
}