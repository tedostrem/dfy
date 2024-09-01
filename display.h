#ifndef DISPLAY_H
#define DISPLAY_H

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>

#define VIDEO_MODE 0
#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240
#define SCREEN_CENTER_X (SCREEN_RES_X >> 1)
#define SCREEN_CENTER_Y (SCREEN_RES_Y >> 1)
#define SCREEN_Z 320

typedef struct {
  DRAWENV draw[2];
  DISPENV disp[2];
} double_buffer;

u_short get_current_buffer(void);

void screen_init(void);
void display_frame(void);

#endif
