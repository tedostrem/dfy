#include "joypad.h"
#include <sys/types.h>

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
