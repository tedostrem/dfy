#include <stdlib.h>
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
#include "geometry.h"

extern char __heap_start, __sp;

typedef enum {
    DRAW_MODE_FLAT,
    DRAW_MODE_WIRE
} draw_mode;

VECTOR light_vector = {4096, -2000, -4096}; // Light coming from the camera direction
VECTOR light_position = {0, 2000, 0};

///////////////////////////////////////////////////////////////////////////////
// Declarations and global variables
///////////////////////////////////////////////////////////////////////////////
POLY_F3 *cube_polys;
POLY_F3 *floor_polys;

MATRIX worldmat = {0};
MATRIX viewmat = {0};

MATRIX cubemat = {0};
MATRIX floormat = {0};
MATRIX cubemat_final = {0};
MATRIX floormat_final = {0};

struct camera cam;

struct object* cube0;
struct object* floor0;

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
        VectorNormal(&normal, &normal);

        // Store the normal in the appropriate place
        m->normals[i / 3].vx = normal.vx;
        m->normals[i / 3].vy = normal.vy;
        m->normals[i / 3].vz = normal.vz;

        printf("Normal for triangle %d: (%ld, %ld, %ld)\n", i / 3, normal.vx, normal.vy, normal.vz);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Render a mesh function
///////////////////////////////////////////////////////////////////////////////
void render_object(struct object *obj, MATRIX *worldmat, draw_mode mode) {
    long distance, intensity, attenuation;
    VECTOR face_center;
    VECTOR light_direction;
    SVECTOR transformed_normal;
    CVECTOR color;
    POLY_F3 *polys;
    LINE_G3 *lines;
    long p, otz, flag;

    for (size_t i = 0; i < obj->mesh->face_count; i += 3) {
        polys = (POLY_F3*)get_next_prim();

        int nclip = RotAverageNclip3(
            &obj->mesh->vertices[obj->mesh->faces[i + 0]],
            &obj->mesh->vertices[obj->mesh->faces[i + 1]],
            &obj->mesh->vertices[obj->mesh->faces[i + 2]],
            (long*)&polys->x0,
            (long*)&polys->x1,
            (long*)&polys->x2,
            &p, &otz, &flag
        );

        if (nclip <= 0 || otz <= 0 || otz >= OT_LEN) {
            continue; // Skip if clipped or out of range
        }

        face_center.vx = (obj->mesh->vertices[obj->mesh->faces[i + 0]].vx +
                  obj->mesh->vertices[obj->mesh->faces[i + 1]].vx +
                  obj->mesh->vertices[obj->mesh->faces[i + 2]].vx) / 3;
        face_center.vy = (obj->mesh->vertices[obj->mesh->faces[i + 0]].vy +
                  obj->mesh->vertices[obj->mesh->faces[i + 1]].vy +
                  obj->mesh->vertices[obj->mesh->faces[i + 2]].vy) / 3;
        face_center.vz = (obj->mesh->vertices[obj->mesh->faces[i + 0]].vz +
                  obj->mesh->vertices[obj->mesh->faces[i + 1]].vz +
                  obj->mesh->vertices[obj->mesh->faces[i + 2]].vz) / 3;

        light_direction.vx = light_position.vx - face_center.vx;
        light_direction.vy = light_position.vy - face_center.vy;
        light_direction.vz = light_position.vz - face_center.vz;

        // Calculate the distance (magnitude of the light direction vector)
        distance = SquareRoot0(light_direction.vx * light_direction.vx +
                               light_direction.vy * light_direction.vy +
                               light_direction.vz * light_direction.vz);    

        // Apply attenuation (inverse-square law or linear scaling)
        attenuation = (4096 * 4096) / ((distance + 1) * (distance + 1));  // Inverse square scaling
        attenuation *= 600;

        // Transform the normal vector using only the object's world matrix
        ApplyMatrixLV(worldmat, &obj->mesh->normals[i / 3], &transformed_normal);

        // Normalize the light direction
        VectorNormal(&light_direction, &light_direction);

        // Intensity calculation based on the transformed normal and light direction
        intensity = (transformed_normal.vx * light_direction.vx +
                     transformed_normal.vy * light_direction.vy +
                     transformed_normal.vz * light_direction.vz) >> 12;

        // Apply attenuation to the intensity based on the distance
        intensity = (intensity * attenuation) >> 12;
        intensity = (intensity < 0) ? 0 : (intensity > 4096) ? 4096 : intensity;

        long ambient = 1138;
        long diffuse = 2458;
        intensity = ambient + ((diffuse * intensity) >> 12);
        intensity = (intensity > 4096) ? 4096 : intensity;

        color.r = (obj->color.r * intensity) >> 12;
        color.g = (obj->color.g * intensity) >> 12;
        color.b = (obj->color.b * intensity) >> 12;

        setPolyF3(polys);
        setRGB0(polys, color.r, color.g, color.b);

        addPrim(get_ot_at(get_current_buffer(), otz), polys);
        increment_next_prim(sizeof(POLY_F3));
    }
}
///////////////////////////////////////////////////////////////////////////////
// Setup function that is called once at the beginning of the execution
///////////////////////////////////////////////////////////////////////////////
void Setup(void) {
    InitHeap3((unsigned long *)&(__heap_start), (&__sp - 0x5000) - &__heap_start);
    cube0 = create_cube();
    floor0 = create_floor();

    calculate_normals(cube0->mesh);
    calculate_normals(floor0->mesh);

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

}

///////////////////////////////////////////////////////////////////////////////
// Update function that is called once per frame
///////////////////////////////////////////////////////////////////////////////
void Update(void) {
    u_long pad;

    // Empty the Ordering Table
    empty_ot(get_current_buffer());

    // Update the state of the controller
    joypad_update();

    // Camera movement based on joypad input
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

    // Update the positions of cube and floor
    cube0->position.vx = 0;
    cube0->position.vy = -100;
    cube0->position.vz = 0;

    floor0->position.vx = 0;
    floor0->position.vy = 0;
    floor0->position.vz = 0;

    // Calculate the transformation matrices for the cube
    ScaleMatrix(&cubemat, &cube0->scale);
    RotMatrix(&cube0->rotation, &cubemat);
    TransMatrix(&cubemat, &cube0->position);

    // Calculate the transformation matrices for the floor
    ScaleMatrix(&floormat, &floor0->scale);
    RotMatrix(&floor0->rotation, &floormat);
    TransMatrix(&floormat, &floor0->position);

    // Create the view matrix from the camera
    camera_look_at(&cam, &cam.position, &cube0->position, &(VECTOR){0, -ONE, 0});

    // Combine camera matrix with the object matrices
    CompMatrixLV(&cam.lookat, &cubemat, &cubemat_final);
    CompMatrixLV(&cam.lookat, &floormat, &floormat_final);

    // Render the cube with its transformed matrix
    SetRotMatrix(&cubemat_final);
    SetTransMatrix(&cubemat_final);
    render_object(cube0, &cubemat_final, DRAW_MODE_FLAT);

    // Render the floor with its transformed matrix
    SetRotMatrix(&floormat_final);
    SetTransMatrix(&floormat_final);
    render_object(floor0, &floormat_final, DRAW_MODE_FLAT);
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
