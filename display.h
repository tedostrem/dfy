#ifndef DISPLAY_H
#define DISPLAY_H

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>

#define VIDEO_MODE 0
#define SCREEN_RES_X 640
#define SCREEN_RES_Y 480
#define SCREEN_CENTER_X (SCREEN_RES_X >> 1)
#define SCREEN_CENTER_Y (SCREEN_RES_Y >> 1)
#define SCREEN_Z 640

struct double_buffer {
  DRAWENV draw[2];
  DISPENV disp[2];
};

u_short get_current_buffer(void);

void screen_init(void);
void display_frame(void);
void draw_screen_borders(CVECTOR background_color, CVECTOR border_color, int border_thickness);

#endif
