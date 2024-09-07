#include "display.h"
#include "globals.h"
#include "libgte.h"

#define WIDESCREEN_HEIGHT 360

CVECTOR background_color = {0, 0, 0};
CVECTOR border_color = {255, 255, 255};

static struct double_buffer screen;           // Struct to hold the display & draw buffers
static u_short currbuff;            // Holds the current buffer number (0 or 1)

u_short get_current_buffer(void) {
  return currbuff;
}

void screen_init(void) {
  // Reset GPU
  ResetGraph(0);

  int border_height = (SCREEN_RES_Y - WIDESCREEN_HEIGHT) / 2;
  SetDefDrawEnv(&screen.draw[0], 0, border_height, SCREEN_RES_X, WIDESCREEN_HEIGHT);
  SetDefDrawEnv(&screen.draw[1], 0, border_height, SCREEN_RES_X, WIDESCREEN_HEIGHT);
  SetDefDispEnv(&screen.disp[0], 0, 0, SCREEN_RES_X, SCREEN_RES_Y);
  SetDefDispEnv(&screen.disp[1], 0, 0, SCREEN_RES_X, SCREEN_RES_Y);


  // Set the back/drawing buffer
  screen.draw[0].isbg = 1;
  screen.draw[1].isbg = 1;

  // Set the background clear color
  setRGB0(&screen.draw[0], 60, 62, 74); // dark purple
  setRGB0(&screen.draw[1], 60, 62, 74); // dark purple

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

  draw_screen_borders(background_color, border_color, 2);
  // Swap current buffer
  currbuff = !currbuff;

  // Reset next primitive pointer to the start of the primitive buffer
  reset_next_prim(currbuff);
}

void draw_screen_borders(CVECTOR background_color, CVECTOR border_color, int border_thickness) {
    int black_border_height = (SCREEN_RES_Y - WIDESCREEN_HEIGHT) / 2;
    int white_border_thickness = border_thickness; // Adjust thickness as needed

    // Black borders
    RECT top_black_border = {0, 0, SCREEN_RES_X, black_border_height};
    RECT bottom_black_border = {0, SCREEN_RES_Y - black_border_height, SCREEN_RES_X, black_border_height};

    // White borders
    RECT top_white_border = {0, black_border_height - white_border_thickness, SCREEN_RES_X, white_border_thickness};
    RECT bottom_white_border = {0, SCREEN_RES_Y - black_border_height, SCREEN_RES_X, white_border_thickness};

    // Clear the black borders
    ClearImage(&top_black_border, background_color.r, background_color.g, background_color.b);
    ClearImage(&bottom_black_border, background_color.r, background_color.g, background_color.b);

    // Draw the white borders
    ClearImage(&top_white_border, border_color.r, border_color.g, border_color.b);
    ClearImage(&bottom_white_border, border_color.r, border_color.g, border_color.b);
}
