#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include "camera.h"

int debug_is_identity_matrix(MATRIX* mat) {
    if (mat->m[0][0] != ONE || mat->m[1][1] != ONE || mat->m[2][2] != ONE)
        return 0;
    if (mat->m[0][1] != 0 || mat->m[0][2] != 0 ||
        mat->m[1][0] != 0 || mat->m[1][2] != 0 ||
        mat->m[2][0] != 0 || mat->m[2][1] != 0)
        return 0;
    return 1;
}

int debug_check_scaling_factors(MATRIX* mat) {
    long diag_1 = mat->m[0][0];
    long diag_2 = mat->m[1][1];
    long diag_3 = mat->m[2][2];

    // Allow a small tolerance for fixed-point precision errors
    long tolerance = ONE >> 10; // You can adjust the tolerance based on your needs

    if (abs(diag_1 - diag_2) > tolerance || abs(diag_1 - diag_3) > tolerance)
        return 0;

    return 1;
}

int debug_check_matrix_determinant(MATRIX* mat) {
    // Calculate the determinant (you can reuse your existing logic)
    long det = (mat->m[0][0] >> 6) * ((mat->m[1][1] >> 6) * (mat->m[2][2] >> 6) - (mat->m[1][2] >> 6) * (mat->m[2][1] >> 6)) -
               (mat->m[0][1] >> 6) * ((mat->m[1][0] >> 6) * (mat->m[2][2] >> 6) - (mat->m[1][2] >> 6) * (mat->m[2][0] >> 6)) +
               (mat->m[0][2] >> 6) * ((mat->m[1][0] >> 6) * (mat->m[2][1] >> 6) - (mat->m[1][1] >> 6) * (mat->m[2][0] >> 6));

    if (det == 0) {
        printf("Warning: Matrix is non-invertible (det == 0)\n");
        return 0;
    }

    // Optionally, check for very small determinants (near singular)
    long min_det = ONE >> 4; // Define your minimum threshold
    if (abs(det) < min_det) {
        printf("Warning: Matrix determinant is very small (%ld)\n", det);
        return 0;
    }

    return 1;
}

int debug_is_normalized(VECTOR* vec) {
    long length_squared = (vec->vx * vec->vx) + (vec->vy * vec->vy) + (vec->vz * vec->vz);

    // Since the length should be `ONE`, compare the squared length with `ONE^2`
    long tolerance = ONE >> 10; // Allow some tolerance for fixed-point errors
    if (abs(length_squared - ONE * ONE) > tolerance) {
        printf("Warning: Vector is not normalized (length_squared = %ld)\n", length_squared);
        return 0;
    }

    return 1;
}

int debug_is_zero_vector(VECTOR* vec) {
    if (vec->vx == 0 && vec->vy == 0 && vec->vz == 0) {
        printf("Warning: Zero-length vector detected\n");
        return 1;
    }
    return 0;
}

int debug_check_vector_range(VECTOR* vec) {
    long max_value = ONE * 10;  // Define a reasonable upper limit for vector components

    if (abs(vec->vx) > max_value || abs(vec->vy) > max_value || abs(vec->vz) > max_value) {
        printf("Warning: Vector component out of range (vx = %ld, vy = %ld, vz = %ld)\n", vec->vx, vec->vy, vec->vz);
        return 0;
    }

    return 1;
}

void debug_matrix(MATRIX *mat) {
    if (!debug_check_matrix_determinant(mat)) {
        printf("Matrix check failed\n");
        printf("%ld %ld %ld\n", mat->m[0][0], mat->m[0][1], mat->m[0][2]);
        printf("%ld %ld %ld\n", mat->m[1][0], mat->m[1][1], mat->m[1][2]);
        printf("%ld %ld %ld\n", mat->m[2][0], mat->m[2][1], mat->m[2][2]);
    }
    
    if (!debug_check_scaling_factors(mat)) {
        printf("Matrix scaling factors are inconsistent\n");
        printf("%ld %ld %ld\n", mat->m[0][0], mat->m[0][1], mat->m[0][2]);
        printf("%ld %ld %ld\n", mat->m[1][0], mat->m[1][1], mat->m[1][2]);
        printf("%ld %ld %ld\n", mat->m[2][0], mat->m[2][1], mat->m[2][2]);
    }
}

void debug_vector(VECTOR* vec) {
    if (!debug_is_normalized(vec)) {
        printf("Vector is not normalized\n");
    }

    if (debug_is_zero_vector(vec)) {
        printf("Zero-length vector found\n");
    }

    if (!debug_check_vector_range(vec)) {
        printf("Vector components are out of range\n");
    }

    printf("Vector: (%ld, %ld, %ld)\n", vec->vx, vec->vy, vec->vz);
}

// Function to print the camera position and matrix
void debug_camera(struct camera *cam) {
    printf("Camera position: (%ld, %ld, %ld)\n", cam->position.vx, cam->position.vy, cam->position.vz);
    printf("Lookat matrix:\n");
    printf("%ld %ld %ld\n", cam->lookat.m[0][0], cam->lookat.m[0][1], cam->lookat.m[0][2]);
    printf("%ld %ld %ld\n", cam->lookat.m[1][0], cam->lookat.m[1][1], cam->lookat.m[1][2]);
    printf("%ld %ld %ld\n", cam->lookat.m[2][0], cam->lookat.m[2][1], cam->lookat.m[2][2]);
    printf("Translation: %ld %ld %ld\n", cam->lookat.t[0], cam->lookat.t[1], cam->lookat.t[2]);
}
