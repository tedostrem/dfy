#ifndef CAMERA_H
#define CAMERA_H

#include "globals.h"

typedef struct camera {
  VECTOR position;
  SVECTOR rotation;
  MATRIX lookat;
} camera;

void LookAt(camera* cam, VECTOR *eye, VECTOR *target, VECTOR *up);

#endif