#include <malloc.h>
#include <string.h>
#include "geometry.h"

SVECTOR floor_vertices[] = {
    // First row (negative Z)
    {-900, 0, -900},  // 0  - Left quad (third)
    {-900, 0, -300},  // 1  - Left quad (third)
    {-300, 0, -900},  // 2  - Middle quad (first)
    {-300, 0, -300},  // 3  - Middle quad (first)

    {-300, 0, -900},  // 4  - Middle quad (first)
    {-300, 0, -300},  // 5  - Middle quad (first)
    { 300, 0, -900},  // 6  - Right quad (second)
    { 300, 0, -300},  // 7  - Right quad (second)

    { 300, 0, -900},  // 8  - Right quad (second)
    { 300, 0, -300},  // 9  - Right quad (second)
    { 900, 0, -900},  // 10 - Right quad (second)
    { 900, 0, -300},  // 11 - Right quad (second)

    // Second row (Z = 0)
    {-900, 0, -300},  // 12 - Left quad (third)
    {-900, 0,  300},  // 13 - Left quad (third)
    {-300, 0, -300},  // 14 - Middle quad (first)
    {-300, 0,  300},  // 15 - Middle quad (first)

    {-300, 0, -300},  // 16 - Middle quad (first)
    {-300, 0,  300},  // 17 - Middle quad (first)
    { 300, 0, -300},  // 18 - Right quad (second)
    { 300, 0,  300},  // 19 - Right quad (second)

    { 300, 0, -300},  // 20 - Right quad (second)
    { 300, 0,  300},  // 21 - Right quad (second)
    { 900, 0, -300},  // 22 - Right quad (second)
    { 900, 0,  300},  // 23 - Right quad (second)

    // Third row (positive Z)
    {-900, 0,  300},  // 24 - Left quad (third)
    {-900, 0,  900},  // 25 - Left quad (third)
    {-300, 0,  300},  // 26 - Middle quad (first)
    {-300, 0,  900},  // 27 - Middle quad (first)

    {-300, 0,  300},  // 28 - Middle quad (first)
    {-300, 0,  900},  // 29 - Middle quad (first)
    { 300, 0,  300},  // 30 - Right quad (second)
    { 300, 0,  900},  // 31 - Right quad (second)

    { 300, 0,  300},  // 32 - Right quad (second)
    { 300, 0,  900},  // 33 - Right quad (second)
    { 900, 0,  300},  // 34 - Right quad (second)
    { 900, 0,  900}   // 35 - Right quad (second)
};

short floor_faces[] = {
    // First row (negative Z)
    0, 1, 2, 3,    // Left quad (third)
    4, 5, 6, 7,    // Middle quad (first)
    8, 9, 10, 11,  // Right quad (second)

    // Second row (Z = 0)
    12, 13, 14, 15,    // Left quad (third)
    16, 17, 18, 19,    // Middle quad (first)
    20, 21, 22, 23,    // Right quad (second)

    // Third row (positive Z)
    24, 25, 26, 27,    // Left quad (third)
    28, 29, 30, 31,    // Middle quad (first)
    32, 33, 34, 35     // Right quad (second)
};


VECTOR floor_normals[9];  // Now 9 normals for 9 quads

struct mesh floor_mesh = {
    floor_vertices,
    floor_faces,
    floor_normals,
    ARRAY_SIZE(floor_vertices),
    ARRAY_SIZE(floor_faces),
    ARRAY_SIZE(floor_normals),
};

CVECTOR floor_color = {200, 200, 255};

struct object object_floor = {
    {0, 0, 0},
    {0, 450, 1800},
    {ONE, ONE, ONE},
    {0, 0, 0},
    {0, 0, 0},
    &floor_mesh,
    {255, 255, 255}
};

struct object* create_floor(void) {
    struct object *o = (struct object*)malloc3(sizeof(struct object));
    memcpy(o, &object_floor, sizeof(struct object));
    return o;
}