#include <malloc.h>
#include <sys/types.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include "debug.h"
#include "inline_n.h"
#include "globals.h"
#include "display.h"
#include "joypad.h"
#include "camera.h"
#include <stdio.h>
#include "geometry.h"
#include "math.h"

extern char __heap_start, __sp;

typedef enum {
    DRAW_MODE_FLAT,
    DRAW_MODE_WIRE
} draw_mode;

VECTOR light_position = {0, 2000, 0};

MATRIX identity = {ONE, 0, 0, 0, ONE, 0, 0, 0, ONE};

// Declarations and global variables
struct camera cam;

struct object* floor0;
POLY_F3 *floor_polys;
MATRIX floormat = {0};
MATRIX floormat_final = {0};

// Render an object function
void render_object(struct object *obj, MATRIX *worldmat, MATRIX *viewmat, draw_mode mode) {
    MATRIX local_world;
    MATRIX local_screen;
    SVECTOR *verts[4];
    long p, otz, flag;
    POLY_F4 *poly;

    // Combine world and view matrices
    MATRIX world_view;
    CompMatrixLV(viewmat, worldmat, &world_view);
    
    // Set the combined world-view matrix for GTE operations
    SetRotMatrix(&world_view);
    SetTransMatrix(&world_view);

    for (size_t i = 0; i < obj->mesh->face_count; i += 4) {
        poly = (POLY_F4*)get_next_prim();

        // Get pointers to the four vertices of this quad
        for (int j = 0; j < 4; j++) {
            verts[j] = &obj->mesh->vertices[obj->mesh->faces[i + j]];
        }

        // Perform perspective transformation
        int nclip = RotAverageNclip4(verts[0], verts[1], verts[2], verts[3],
                                     (long*)&poly->x0, (long*)&poly->x1,
                                     (long*)&poly->x2, (long*)&poly->x3,
                                     &p, &otz, &flag);

        if (nclip <= 0 || otz <= 0 || otz >= OT_LEN) continue;

        // Set up the polygon for drawing
        setPolyF4(poly);
        setRGB0(poly, obj->color.r, obj->color.g, obj->color.b);

        // Add the polygon to the ordering table
        addPrim(get_ot_at(get_current_buffer(), otz), poly);
        increment_next_prim(sizeof(POLY_F4));
    }
}

// Main function
int main(void) {
    // Initialize the memory heap
    InitHeap3((unsigned long *)&(__heap_start), (&__sp - 0x8000) - &__heap_start);

    // Create the floor object and calculate normals
    floor0 = create_floor();
    math_calculate_normals(floor0->mesh);

    // Setup the display environment
    screen_init();

    // Initialize the joypad
    joypad_init();

    // Initialize floor's rotation and position
    floor0->rotation.vx = 0;
    floor0->rotation.vy = 0;
    floor0->rotation.vz = 0;

    floor0->position.vx = 0;
    floor0->position.vy = 0;
    floor0->position.vz = 0;

    // Initialize the camera object
    cam.position.vx = 0;
    cam.position.vy = -300;
    cam.position.vz = -2000; // Move the camera farther away

    cam.rotation.vx = 0;
    cam.rotation.vy = 0;
    cam.rotation.vz = 0;
    cam.lookat = identity;

    // Reset next primitive pointer to the start of the primitive buffer
    reset_next_prim(get_current_buffer());

    while (1) {
        // Empty the Ordering Table
        empty_ot(get_current_buffer());

        // Update camera from joypad and create the view matrix from the camera
        joypad_update_camera_position(&cam.position);
        camera_look_at(&cam, &cam.position, &(VECTOR){0, 0, 0}, &(VECTOR){0, -ONE, 0});
        MatrixNormal(&cam.lookat, &cam.lookat);

        // Calculate the transformation matrices for the floor
        RotMatrix(&floor0->rotation, &floormat);
        TransMatrix(&floormat, &floor0->position);

        // Combine camera matrix with the object matrices
        CompMatrixLV(&cam.lookat, &floormat, &floormat_final);
        math_normalize_matrix(&floormat_final, &floormat_final);

        // Set the rotation and translation matrices for rendering
        SetRotMatrix(&floormat_final);
        SetTransMatrix(&floormat_final);

        // Render the floor with its transformed matrix
        render_object(floor0, &floormat, &floormat_final, DRAW_MODE_FLAT);

        // Display the frame
        display_frame();
    }
    return 0;
}
