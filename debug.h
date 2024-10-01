#ifndef DEBUG_H
#define DEBUG_H
#include <libgte.h>
#include "camera.h"

int debug_is_identity_matrix(MATRIX* mat);
int debug_check_scaling_factors(MATRIX* mat);
int debug_check_matrix_determinant(MATRIX* mat);
int debug_is_normalized(VECTOR* vec);
int debug_is_zero_vector(VECTOR* vec);
int debug_check_vector_range(VECTOR* vec);

void debug_matrix(MATRIX* mat);
void debug_vector(VECTOR* vec);

void debug_camera(struct camera *cam);

#endif