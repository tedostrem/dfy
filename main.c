#include <sys/types.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include "inline_n.h"
#include "globals.h"
#include "display.h"
#include "joypad.h"
#include "camera.h"

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
    { -128, -128, -128 },
    {  128, -128, -128 },
    {  128, -128,  128 },
    { -128, -128,  128 },
    { -128,  128, -128 },
    {  128,  128, -128 },
    {  128,  128,  128 },
    { -128,  128,  128 },
  },
  {
    3, 2, 0, 1,
    0, 1, 4, 5,
    4, 5, 7, 6,
    1, 2, 5, 6,
    2, 3, 6, 7,
    3, 0, 7, 4,
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
  cam.position.vx = 300;
  cam.position.vy = -1000;
  cam.position.vz = -1000;
  cam.lookat = (MATRIX){0};
}

///////////////////////////////////////////////////////////////////////////////
// Setup function that is called once at the beginning of the execution
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
  RotMatrix(&cube0.rotation, &worldmat);
  TransMatrix(&worldmat, &cube0.position);
  ScaleMatrix(&worldmat, &cube0.scale);

  // Create the View Matrix combining the world matrix & lookat matrix
  CompMatrixLV(&cam.lookat, &worldmat, &viewmat);

  SetRotMatrix(&viewmat);
  SetTransMatrix(&viewmat);

  for (i = 0; i < 24; i += 4) {
    cube_polys = (POLY_F4*) get_next_prim();
    setPolyF4(cube_polys);
    setRGB0(cube_polys, 255, 121, 198);

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

    if(nclip < 0) continue;

    if ((otz > 0) && (otz < OT_LEN)) {
      addPrim(get_ot_at(get_current_buffer(), otz), cube_polys);
      increment_next_prim(sizeof(POLY_G4));
    }
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

  for (i = 0; i < 6; i += 3) {
    floor_polys = (POLY_F3*) get_next_prim();
    setPolyF3(floor_polys);
    setRGB0(floor_polys, 241, 250, 140);

    nclip = RotAverageNclip3(
      &floor0.vertices[floor0.faces[i + 0]],
      &floor0.vertices[floor0.faces[i + 1]],
      &floor0.vertices[floor0.faces[i + 2]],
      (long*)&floor_polys->x0,
      (long*)&floor_polys->x1,
      (long*)&floor_polys->x2,
      &p, &otz, &flg
    );

    if (nclip < 0) continue;

    if ((otz > 0) && (otz < OT_LEN)) {
      addPrim(get_ot_at(get_current_buffer(), otz), floor_polys);
      increment_next_prim(sizeof(POLY_F3));
    }
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
