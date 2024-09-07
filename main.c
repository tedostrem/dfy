#include <sys/types.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include "inline_n.h"
#include "globals.h"
#include "display.h"
#include "joypad.h"
#include "camera.h"
#include <stdio.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))
#define CUBESIZE 196

///////////////////////////////////////////////////////////////////////////////
// Vertices and face indices
///////////////////////////////////////////////////////////////////////////////
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
};

VECTOR light_vector = {0, 0, -4096}; // Light coming from the camera direction

///////////////////////////////////////////////////////////////////////////////
// Cube vertices, faces, and normals
///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
// Floor vertices, faces, and normals
///////////////////////////////////////////////////////////////////////////////
SVECTOR floor_vertices[] = {
    {-900, 0, -900},
    {-900, 0,  900},
    { 900, 0, -900},
    { 900, 0,  900},
};

short floor_faces[] = {
    0, 1, 2,
    1, 3, 2,
};

SVECTOR floor_normals[2]; // Normals for the floor faces

///////////////////////////////////////////////////////////////////////////////
// Global Mesh Instances
///////////////////////////////////////////////////////////////////////////////
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
    ARRAY_SIZE(floor_normals)
};

///////////////////////////////////////////////////////////////////////////////
// Declarations and global variables
///////////////////////////////////////////////////////////////////////////////
POLY_F3 *cube_polys;
POLY_F3 *floor_polys;

MATRIX worldmat = {0};
MATRIX viewmat = {0};

struct camera cam;

struct object cube0 = {
    {0, 0, 0},
    {0, -400, 1800},
    {ONE, ONE, ONE},
    {0, 0, 0},
    {0, 1, 0},
    &cube_mesh
};

struct object floor0 = {
    {0, 0, 0},
    {0, 450, 1800},
    {ONE, ONE, ONE},
    {0, 0, 0},
    {0, 0, 0},
    &floor_mesh
};

///////////////////////////////////////////////////////////////////////////////
// Utility functions for vector normalization and normal calculation
///////////////////////////////////////////////////////////////////////////////
static void normalize_vector(SVECTOR *v) {
    long length = SquareRoot0((v->vx * v->vx) + (v->vy * v->vy) + (v->vz * v->vz));
    if (length != 0) {
        v->vx = (v->vx << 12) / length;
        v->vy = (v->vy << 12) / length;
        v->vz = (v->vz << 12) / length;
    }
}

void calculate_normals(struct mesh *m) {
    for (size_t i = 0; i < m->face_count; i += 3) {  // Increment by 3 for each triangle
        VECTOR v0, v1, normal;

        // Calculate two vectors from the three vertices of the triangle
        v0.vx = m->vertices[m->faces[i + 1]].vx - m->vertices[m->faces[i]].vx;
        v0.vy = m->vertices[m->faces[i + 1]].vy - m->vertices[m->faces[i]].vy;
        v0.vz = m->vertices[m->faces[i + 1]].vz - m->vertices[m->faces[i]].vz;

        v1.vx = m->vertices[m->faces[i + 2]].vx - m->vertices[m->faces[i]].vx;
        v1.vy = m->vertices[m->faces[i + 2]].vy - m->vertices[m->faces[i]].vy;
        v1.vz = m->vertices[m->faces[i + 2]].vz - m->vertices[m->faces[i]].vz;

        // Calculate normal as the cross product of v0 and v1
        normal.vx = v0.vy * v1.vz - v0.vz * v1.vy;
        normal.vy = v0.vz * v1.vx - v0.vx * v1.vz;
        normal.vz = v0.vx * v1.vy - v0.vy * v1.vx;

        // Normalize the normal vector
        normalize_vector((SVECTOR*)&normal);

        // Store the normal in the appropriate place
        m->normals[i / 3].vx = normal.vx;
        m->normals[i / 3].vy = normal.vy;
        m->normals[i / 3].vz = normal.vz;

        printf("Normal for triangle %d: (%d, %d, %d)\n", i / 3, normal.vx, normal.vy, normal.vz);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Setup function that is called once at the beginning of the execution
///////////////////////////////////////////////////////////////////////////////
void Setup(void) {
    // Setup the display environment
    screen_init();

    // Initializes the joypad
    joypad_init();

    // Reset next primitive pointer to the start of the primitive buffer
    reset_next_prim(get_current_buffer());

    // Initializes the camera object
    cam.position.vx = 0;
    cam.position.vy = -300;
    cam.position.vz = -500; // Move the camera farther away
    cam.lookat = (MATRIX){0};

    // Calculate normals for both the cube and the floor
    calculate_normals(cube0.mesh);
    calculate_normals(floor0.mesh);
}

///////////////////////////////////////////////////////////////////////////////
// Update function that is called once per frame
///////////////////////////////////////////////////////////////////////////////
void Update(void) {
    int i, nclip;
    long otz, p, flg;
    u_long pad;

    // Empty the Ordering Table
    empty_ot(get_current_buffer());

    // Update the state of the controller
    joypad_update();

    if (joypad_check(PAD1_LEFT)) {
        cam.position.vx -= 50;
    }
    if (joypad_check(PAD1_RIGHT)) {
        cam.position.vx += 50;
    }
    if (joypad_check(PAD1_UP)) {
        cam.position.vy -= 50;
    }
    if (joypad_check(PAD1_DOWN)) {
        cam.position.vy += 50;
    }
    if (joypad_check(PAD1_CROSS)) {
        cam.position.vz += 50;
    }
    if (joypad_check(PAD1_CIRCLE)) {
        cam.position.vz -= 50;
    }

    // Update the cube velocity based on its acceleration
    cube0.vel.vx += cube0.acc.vx;
    cube0.vel.vy += cube0.acc.vy;
    cube0.vel.vz += cube0.acc.vz;

    // Update the cube position based on its velocity
    cube0.position.vx += (cube0.vel.vx) >> 1;
    cube0.position.vy += (cube0.vel.vy) >> 1;
    cube0.position.vz += (cube0.vel.vz) >> 1;

    // Check "collision" with the floor
    if (cube0.position.vy + 200 > floor0.position.vy) {
        cube0.vel.vy *= -1;
    }

    // Compute the camera Lookat matrix for this frame
    camera_look_at(&cam, &cam.position, &cube0.position, &(VECTOR){0, -ONE, 0});

    /////////////////////
    // Draw the Cube
    /////////////////////
    ScaleMatrix(&worldmat, &cube0.scale);
    RotMatrix(&cube0.rotation, &worldmat);
    TransMatrix(&worldmat, &cube0.position);

    // Create the View Matrix combining the world matrix & lookat matrix
    CompMatrixLV(&cam.lookat, &worldmat, &viewmat);

    SetRotMatrix(&viewmat);
    SetTransMatrix(&viewmat);

    VECTOR transformed_normal;
    CVECTOR color;
    long intensity;

    for (i = 0; i < cube0.mesh->face_count; i += 3) {
        cube_polys = (POLY_F3*) get_next_prim();

        nclip = RotAverageNclip3(
            &cube0.mesh->vertices[cube0.mesh->faces[i + 0]],
            &cube0.mesh->vertices[cube0.mesh->faces[i + 1]],
            &cube0.mesh->vertices[cube0.mesh->faces[i + 2]],
            (long*)&cube_polys->x0,
            (long*)&cube_polys->x1,
            (long*)&cube_polys->x2,
            &p, &otz, &flg
        );

        ApplyMatrix(&viewmat, &cube0.mesh->normals[i / 3], &transformed_normal);

        // Normalize the transformed normal to avoid scaling issues
        normalize_vector((SVECTOR*)&transformed_normal);

        // Calculate dot product for lighting
        intensity = (transformed_normal.vx * light_vector.vx +
                     transformed_normal.vy * light_vector.vy +
                     transformed_normal.vz * light_vector.vz) >> 12;

        // Ensure intensity is positive and normalize it 
        intensity = (intensity < 0) ? 0 : (intensity > 4096) ? 4096 : intensity;

        // Apply lighting calculation 
        long ambient = 1138; // ~40% ambient light (4096 * 0.4) 
        long diffuse = 2458; // ~60% diffuse light (4096 * 0.6) 

        // Combine ambient and diffuse lighting 
        intensity = ambient + ((diffuse * intensity) >> 12);

        // Ensure final intensity doesn't exceed 4096 (full brightness) 
        intensity = (intensity > 4096) ? 4096 : intensity;

        color.r = (255 * intensity) >> 12;
        color.g = (121 * intensity) >> 12;
        color.b = (198 * intensity) >> 12;

        setPolyF3(cube_polys);
        setRGB0(cube_polys, color.r, color.g, color.b);

        if (nclip <= 0 || otz <= 0 || otz >= OT_LEN) continue;
        
        addPrim(get_ot_at(get_current_buffer(), otz), cube_polys);
        increment_next_prim(sizeof(POLY_F4));
    }

    /////////////////////
    // Draw the Floor
    /////////////////////
    RotMatrix(&floor0.rotation, &worldmat);
    TransMatrix(&worldmat, &floor0.position);
    ScaleMatrix(&worldmat, &floor0.scale);

    // Create the View Matrix combining the world matrix & lookat matrix
    CompMatrixLV(&cam.lookat, &worldmat, &viewmat);

    SetRotMatrix(&viewmat);
    SetTransMatrix(&viewmat);

    for (i = 0; i < floor0.mesh->face_count; i += 3) {
        floor_polys = (POLY_F3*) get_next_prim();

        nclip = RotAverageNclip3(
            &floor0.mesh->vertices[floor0.mesh->faces[i + 0]],
            &floor0.mesh->vertices[floor0.mesh->faces[i + 1]],
            &floor0.mesh->vertices[floor0.mesh->faces[i + 2]],
            (long*)&floor_polys->x0,
            (long*)&floor_polys->x1,
            (long*)&floor_polys->x2,
            &p, &otz, &flg
        );

        ApplyMatrix(&viewmat, &floor0.mesh->normals[i / 3], &transformed_normal);

        // Normalize the transformed normal to avoid scaling issues
        normalize_vector((SVECTOR*)&transformed_normal);

        // Calculate dot product for lighting
        intensity = (transformed_normal.vx * light_vector.vx +
                     transformed_normal.vy * light_vector.vy +
                     transformed_normal.vz * light_vector.vz) >> 12;

        // Ensure intensity is positive and normalize it 
        intensity = (intensity < 0) ? 0 : (intensity > 4096) ? 4096 : intensity;

        // Apply lighting calculation 
        long ambient = 1138; // ~40% ambient light 
        long diffuse = 2458; // ~60% diffuse light 

        // Combine ambient and diffuse lighting 
        intensity = ambient + ((diffuse * intensity) >> 12);

        // Ensure final intensity doesn't exceed 4096 (full brightness) 
        intensity = (intensity > 4096) ? 4096 : intensity;

        color.r = (241 * intensity) >> 12;
        color.g = (250 * intensity) >> 12;
        color.b = (140 * intensity) >> 12;

        setPolyF3(floor_polys);
        setRGB0(floor_polys, color.r, color.g, color.b);

        if (nclip <= 0 || otz <= 0 || otz >= OT_LEN) continue;

        addPrim(get_ot_at(get_current_buffer(), otz), floor_polys);
        increment_next_prim(sizeof(POLY_F3));
    }
}

///////////////////////////////////////////////////////////////////////////////
// Render function that invokes the draw of the current frame
///////////////////////////////////////////////////////////////////////////////
void Render(void) {
    display_frame();
}

///////////////////////////////////////////////////////////////////////////////
// Main function
///////////////////////////////////////////////////////////////////////////////
int main(void) {

    Setup();
    while (1) {
        Update();
        Render();
    }
    return 0;
}
