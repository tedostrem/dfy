#include "globals.h"

static u_long ot[2][OT_LEN];            // Ordering table holding pointers to sorted primitives
static char primbuff[2][PRIMBUFF_LEN];  // Primitive buffer that holds the actual data for each primitive
static char *nextprim;                  // Pointer to the next primitive in the primitive buffer

void empty_ot(u_short currbuff) {
  ClearOTagR(ot[currbuff], OT_LEN);
}

void set_ot_at(u_short currbuff, u_int i, u_long value) {
  ot[currbuff][i] = value;
}

u_long *get_ot_at(u_short currbuff, u_int i) {
  return &ot[currbuff][i];
}

void increment_next_prim(u_int size) {
  nextprim += size;
}

void reset_next_prim(u_short currbuff) {
  nextprim = primbuff[currbuff];
}

void set_next_prim(char *value) {
  nextprim = value;
}

char *get_next_prim(void) {
  return nextprim;
}
