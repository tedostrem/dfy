#ifndef CAMERA_H
#define CAMERA_H

#include "globals.h"

struct camera {
  VECTOR position;
  SVECTOR rotation;
  MATRIX lookat;
};

void camera_look_at(struct camera* cam, VECTOR *eye, VECTOR *target, VECTOR *up);

#endif