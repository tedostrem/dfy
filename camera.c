#include "camera.h"

void vector_cross(VECTOR *a, VECTOR *b, VECTOR *out) {
  OuterProduct12(a, b, out);
}

void camera_look_at(struct camera *cam, VECTOR *eye, VECTOR *target, VECTOR *up) {
  VECTOR xright;
  VECTOR yup;
  VECTOR zforward;
  VECTOR x, y, z;
  VECTOR pos;
  VECTOR t;

  zforward.vx = target->vx - eye->vx;
  zforward.vy = target->vy - eye->vy;
  zforward.vz = target->vz - eye->vz;
  VectorNormal(&zforward, &z);

  vector_cross(&z, up, &xright);
  VectorNormal(&xright, &x);

  vector_cross(&z, &x, &yup);
  VectorNormal(&yup, &y);

  cam->lookat.m[0][0] = x.vx; cam->lookat.m[0][1] = x.vy; cam->lookat.m[0][2] = x.vz;
  cam->lookat.m[1][0] = y.vx; cam->lookat.m[1][1] = y.vy; cam->lookat.m[1][2] = y.vz;
  cam->lookat.m[2][0] = z.vx; cam->lookat.m[2][1] = z.vy; cam->lookat.m[2][2] = z.vz;

  pos.vx = -eye->vx;
  pos.vy = -eye->vy;
  pos.vz = -eye->vz;

  ApplyMatrixLV(&cam->lookat, &pos, &t);
  TransMatrix(&cam->lookat, &t);
}
