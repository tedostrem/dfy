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

VECTOR light_vector = {0, 0, -4096}; // Light coming from the camera direction

///////////////////////////////////////////////////////////////////////////////
// Vertices and face indices
///////////////////////////////////////////////////////////////////////////////
struct cube {
  SVECTOR rotation;
  VECTOR position;
  VECTOR scale;
  VECTOR vel;
  VECTOR acc;
  SVECTOR vertices[8];
  short faces[24];
  SVECTOR normals[6];
};

struct floor {
  SVECTOR rotation;
  VECTOR position;
  VECTOR scale;
  SVECTOR vertices[4];
  short faces[6];
};

///////////////////////////////////////////////////////////////////////////////
// Declarations and global variables
///////////////////////////////////////////////////////////////////////////////
POLY_F4 *cube_polys;
POLY_F3 *floor_polys;

MATRIX worldmat = {0};
MATRIX viewmat = {0};

struct camera cam;

struct cube cube0 = {
  {0, 0, 0},
  {0, -400, 1800},
  {ONE, ONE, ONE},
  {0, 0, 0},
  {0, 1, 0},
{
  {-CUBESIZE / 2, -CUBESIZE / 2, -CUBESIZE / 2, 0},
  {CUBESIZE / 2, -CUBESIZE / 2, -CUBESIZE / 2, 0},
  {CUBESIZE / 2, CUBESIZE / 2, -CUBESIZE / 2, 0},
  {-CUBESIZE / 2, CUBESIZE / 2, -CUBESIZE / 2, 0},
  {-CUBESIZE / 2, -CUBESIZE / 2, CUBESIZE / 2, 0},
  {CUBESIZE / 2, -CUBESIZE / 2, CUBESIZE / 2, 0},
  {CUBESIZE / 2, CUBESIZE / 2, CUBESIZE / 2, 0},
  {-CUBESIZE / 2, CUBESIZE / 2, CUBESIZE / 2, 0}
},
  {
        0, 1, 2, 3, // Front face
        1, 5, 6, 2, // Right face
        5, 4, 7, 6, // Back face
        4, 0, 3, 7, // Left face
        4, 5, 1, 0, // Bottom face
        6, 7, 3, 2  // Top face
    }
};

struct floor floor0 = {
  {0, 0, 0},
  {0, 450, 1800},
  {ONE, ONE, ONE},
  {
    {-900, 0, -900},
    {-900, 0,  900},
    { 900, 0, -900},
    { 900, 0,  900},
  },
  {
    0, 1, 2,
    1, 3, 2,
  }
};

static void normalize_vector(SVECTOR *v) {
    long length = SquareRoot0((v->vx * v->vx) + (v->vy * v->vy) + (v->vz * v->vz));
    if (length != 0) {
        v->vx = (v->vx << 12) / length;
        v->vy = (v->vy << 12) / length;
        v->vz = (v->vz << 12) / length;
    }
}

void cube_calculate_normals(struct cube *c) {
    for (size_t i = 0; i < ARRAY_SIZE(c->faces); i += 4) {
        VECTOR v0, v1, normal;
        v0.vx = c->vertices[c->faces[i + 1]].vx - c->vertices[c->faces[i]].vx;
        v0.vy = c->vertices[c->faces[i + 1]].vy - c->vertices[c->faces[i]].vy;
        v0.vz = c->vertices[c->faces[i + 1]].vz - c->vertices[c->faces[i]].vz;
        v1.vx = c->vertices[c->faces[i + 2]].vx - c->vertices[c->faces[i]].vx;
        v1.vy = c->vertices[c->faces[i + 2]].vy - c->vertices[c->faces[i]].vy;
        v1.vz = c->vertices[c->faces[i + 2]].vz - c->vertices[c->faces[i]].vz;

        // Calculate normal as the cross product of v0 and v1 using VECTOR
        normal.vx = v0.vy * v1.vz - v0.vz * v1.vy;
        normal.vy = v0.vz * v1.vx - v0.vx * v1.vz;
        normal.vz = v0.vx * v1.vy - v0.vy * v1.vx;

        // Convert back to SVECTOR if necessary, or keep as VECTOR
        c->normals[i / 4].vx = normal.vx;
        c->normals[i / 4].vy = normal.vy;
        c->normals[i / 4].vz = normal.vz;

        normalize_vector(&c->normals[i / 4]);
        printf("Normal: (%d, %d, %d)\n", c->normals[i / 4].vx, c->normals[i / 4].vy, c->normals[i / 4].vz);
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
  cam.position.vy = -ONE>>1;
  cam.position.vz = -ONE>>1; // Move the camera farther away
  cam.lookat = (MATRIX){0};

  cube_calculate_normals(&cube0);
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

  for (i = 0; i < 24; i += 4) {
    cube_polys = (POLY_F4*) get_next_prim();

    nclip = RotAverageNclip4(
      &cube0.vertices[cube0.faces[i + 0]],
      &cube0.vertices[cube0.faces[i + 1]],
      &cube0.vertices[cube0.faces[i + 2]],
      &cube0.vertices[cube0.faces[i + 3]],
      (long*)&cube_polys->x0,
      (long*)&cube_polys->x1,
      (long*)&cube_polys->x2,
      (long*)&cube_polys->x3,
      &p, &otz, &flg
    );

    ApplyMatrix(&viewmat, &cube0.normals[i / 4], &transformed_normal);

    if(nclip <= 0) continue;


    // Normalize the transformed normal to avoid scaling issues
    normalize_vector((SVECTOR*)&transformed_normal);

    // Calculate dot product for lighting
    intensity = (transformed_normal.vx * cam.position.vx +
                 transformed_normal.vy * cam.position.vy +
                 transformed_normal.vz * cam.position.vz) >> 12;

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

    setPolyF4(cube_polys);
    setRGB0(cube_polys, color.r, color.g, color.b);

    if (nclip <= 0 || otz <= 0 || otz >= OT_LEN) continue;
    
    addPrim(get_ot_at(get_current_buffer(), otz), cube_polys);
    increment_next_prim(sizeof(POLY_F4));
  }

  /////////////////////
  // Draw the Floor
  /////////////////////
  //RotMatrix(&floor0.rotation, &worldmat);
  //TransMatrix(&worldmat, &floor0.position);
  //ScaleMatrix(&worldmat, &floor0.scale);

  //// Create the View Matrix combining the world matrix & lookat matrix
  //CompMatrixLV(&cam.lookat, &worldmat, &viewmat);

  //SetRotMatrix(&viewmat);
  //SetTransMatrix(&viewmat);

  //for (i = 0; i < 6; i += 3) {
  //  floor_polys = (POLY_F3*) get_next_prim();
  //  setPolyF3(floor_polys);
  //  setRGB0(floor_polys, 241, 250, 140);

  //  nclip = RotAverageNclip3(
  //    &floor0.vertices[floor0.faces[i + 0]],
  //    &floor0.vertices[floor0.faces[i + 1]],
  //    &floor0.vertices[floor0.faces[i + 2]],
  //    (long*)&floor_polys->x0,
  //    (long*)&floor_polys->x1,
  //    (long*)&floor_polys->x2,
  //    &p, &otz, &flg
  //  );

  //  if (nclip <= 0 || otz <= 0 || otz >= OT_LEN) continue;

  //  addPrim(get_ot_at(get_current_buffer(), otz), floor_polys);
  //  increment_next_prim(sizeof(POLY_F3));
  //}
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
