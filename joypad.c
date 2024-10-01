#include "joypad.h"
#include <sys/types.h>
#include <libgte.h>

static u_long padstate;

int joypad_check(int p) {
  return padstate & p;
}

void joypad_init(void) {
  PadInit(0);
}

void joypad_reset(void) {
  padstate = 0;
}

void joypad_update(void) {
  u_long pad = PadRead(0);
  padstate = pad;
}

void joypad_update_camera_position(VECTOR *campos) {
    // Update the state of the controller
    joypad_update();

    // Camera movement based on joypad input
    if (joypad_check(PAD1_LEFT)) {
        campos->vx -= 50;
    }
    if (joypad_check(PAD1_RIGHT)) {
        campos->vx += 50;
    }
    if (joypad_check(PAD1_UP)) {
        campos->vy -= 50;
    }
    if (joypad_check(PAD1_DOWN)) {
        campos->vy += 50;
    }
    if (joypad_check(PAD1_CROSS)) {
        campos->vz += 50;
    }
    if (joypad_check(PAD1_CIRCLE)) {
        campos->vz -= 50;
    }
}
