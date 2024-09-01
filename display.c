#include "display.h"
#include "globals.h"

static double_buffer screen;           // Struct to hold the display & draw buffers
static u_short currbuff;            // Holds the current buffer number (0 or 1)

u_short get_current_buffer(void) {
  return currbuff;
}

void screen_init(void) {
  // Reset GPU
  ResetGraph(0);

  // Set the display area of the first buffer
  SetDefDispEnv(&screen.disp[0], 0,   0, SCREEN_RES_X, SCREEN_RES_Y);
  SetDefDrawEnv(&screen.draw[0], 0, 240, SCREEN_RES_X, SCREEN_RES_Y);

  // Set the display area of the second buffer
  SetDefDispEnv(&screen.disp[1], 0, 240, SCREEN_RES_X, SCREEN_RES_Y);
  SetDefDrawEnv(&screen.draw[1], 0,   0, SCREEN_RES_X, SCREEN_RES_Y);

  // Set the back/drawing buffer
  screen.draw[0].isbg = 1;
  screen.draw[1].isbg = 1;

  // Set the background clear color
  setRGB0(&screen.draw[0], 63, 0, 127); // dark purple
  setRGB0(&screen.draw[1], 63, 0, 127); // dark purple

  // Set the current initial buffer
  currbuff = 0;
  PutDispEnv(&screen.disp[currbuff]);
  PutDrawEnv(&screen.draw[currbuff]);

  // Initialize and setup the GTE geometry offsets
  InitGeom();
  SetGeomOffset(SCREEN_CENTER_X, SCREEN_CENTER_Y);
  SetGeomScreen(SCREEN_Z);

  // Enable display
  SetDispMask(1);
}

///////////////////////////////////////////////////////////////////////////////
// Draw the current frame primitives in the ordering table
///////////////////////////////////////////////////////////////////////////////
void display_frame(void) {
  // Sync and wait for vertical blank
  DrawSync(0);
  VSync(0);

  // Set the current display & draw buffers
  PutDispEnv(&screen.disp[currbuff]);
  PutDrawEnv(&screen.draw[currbuff]);

  // Draw the ordering table for the current buffer
  DrawOTag(get_ot_at(currbuff, OT_LEN - 1));

  // Swap current buffer
  currbuff = !currbuff;

  // Reset next primitive pointer to the start of the primitive buffer
  reset_next_prim(currbuff);
}
