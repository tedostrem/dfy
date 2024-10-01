#ifndef MATH_H
#define MATH_H

#include <libgte.h>
#include "geometry.h"


void math_normalize_matrix(MATRIX* src, MATRIX* dst);
void math_invert_matrix(MATRIX* src, MATRIX* dst);
void math_calculate_normals(struct mesh *m);
#endif