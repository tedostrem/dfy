#include <libgte.h>
#include <stdio.h>
#include "geometry.h"
#include "math.h"

void math_normalize_matrix(MATRIX* src, MATRIX* dst) {
    // Step 1: Calculate the length (magnitude) of each row (scaling factors)
    long scale_x = SquareRoot0((src->m[0][0] * src->m[0][0]) + (src->m[0][1] * src->m[0][1]) + (src->m[0][2] * src->m[0][2]));
    long scale_y = SquareRoot0((src->m[1][0] * src->m[1][0]) + (src->m[1][1] * src->m[1][1]) + (src->m[1][2] * src->m[1][2]));
    long scale_z = SquareRoot0((src->m[2][0] * src->m[2][0]) + (src->m[2][1] * src->m[2][1]) + (src->m[2][2] * src->m[2][2]));

    // Step 2: Calculate the average scale factor across all axes
    long avg_scale = (scale_x + scale_y + scale_z) / 3;

    // Step 3: Normalize each row using the average scaling factor
    for (int i = 0; i < 3; i++) {
        dst->m[i][0] = (src->m[i][0] * ONE) / avg_scale;
        dst->m[i][1] = (src->m[i][1] * ONE) / avg_scale;
        dst->m[i][2] = (src->m[i][2] * ONE) / avg_scale;
    }

    // Step 4: Copy the translation part (if applicable)
    dst->t[0] = src->t[0];
    dst->t[1] = src->t[1];
    dst->t[2] = src->t[2];
}


void math_invert_matrix(MATRIX* src, MATRIX* dst) {
    // Calculate the determinant of the 3x3 matrix with more bit shifts
    long det = (src->m[0][0] >> 6) * ((src->m[1][1] >> 6) * (src->m[2][2] >> 6) - (src->m[1][2] >> 6) * (src->m[2][1] >> 6)) -
               (src->m[0][1] >> 6) * ((src->m[1][0] >> 6) * (src->m[2][2] >> 6) - (src->m[1][2] >> 6) * (src->m[2][0] >> 6)) +
               (src->m[0][2] >> 6) * ((src->m[1][0] >> 6) * (src->m[2][1] >> 6) - (src->m[1][1] >> 6) * (src->m[2][0] >> 6));

    // If determinant is zero, the matrix is not invertible
    if (det == 0) {
        printf("Matrix is not invertible\n");
        return;
    }

    // Calculate the inverse of the determinant with a larger scaling factor
    long invDet = (1 << 20) / det;  // Use a fixed-point scaling factor (shifted by 20 bits)
    printf("Determinant: %ld, Inverse determinant: %ld\n", det, invDet);

    // Calculate the adjugate matrix (transpose of cofactor matrix) and apply scaling
    dst->m[0][0] = ((src->m[1][1] * src->m[2][2] - src->m[1][2] * src->m[2][1]) * invDet) >> 20;
    dst->m[0][1] = ((src->m[0][2] * src->m[2][1] - src->m[0][1] * src->m[2][2]) * invDet) >> 20;
    dst->m[0][2] = ((src->m[0][1] * src->m[1][2] - src->m[0][2] * src->m[1][1]) * invDet) >> 20;

    dst->m[1][0] = ((src->m[1][2] * src->m[2][0] - src->m[1][0] * src->m[2][2]) * invDet) >> 20;
    dst->m[1][1] = ((src->m[0][0] * src->m[2][2] - src->m[0][2] * src->m[2][0]) * invDet) >> 20;
    dst->m[1][2] = ((src->m[0][2] * src->m[1][0] - src->m[0][0] * src->m[1][2]) * invDet) >> 20;

    dst->m[2][0] = ((src->m[1][0] * src->m[2][1] - src->m[1][1] * src->m[2][0]) * invDet) >> 20;
    dst->m[2][1] = ((src->m[0][1] * src->m[2][0] - src->m[0][0] * src->m[2][1]) * invDet) >> 20;
    dst->m[2][2] = ((src->m[0][0] * src->m[1][1] - src->m[0][1] * src->m[1][0]) * invDet) >> 20;

    // Copy the translation part (if needed for 4x3 matrix)
    dst->t[0] = src->t[0];
    dst->t[1] = src->t[1];
    dst->t[2] = src->t[2];
}



void math_calculate_normals(struct mesh *m) {
    for (size_t i = 0; i < m->face_count; i += 4) {  // Increment by 4 for each quad
        VECTOR v0, v1, normal;

        // Calculate two vectors from the four vertices of the quad
        v0.vx = m->vertices[m->faces[i + 1]].vx - m->vertices[m->faces[i]].vx;
        v0.vy = m->vertices[m->faces[i + 1]].vy - m->vertices[m->faces[i]].vy;
        v0.vz = m->vertices[m->faces[i + 1]].vz - m->vertices[m->faces[i]].vz;

        v1.vx = m->vertices[m->faces[i + 2]].vx - m->vertices[m->faces[i]].vx;
        v1.vy = m->vertices[m->faces[i + 2]].vy - m->vertices[m->faces[i]].vy;
        v1.vz = m->vertices[m->faces[i + 2]].vz - m->vertices[m->faces[i]].vz;

        // Calculate normal as the cross product of v0 and v1 (based on 3 vertices of the quad)
        normal.vx = v0.vy * v1.vz - v0.vz * v1.vy;
        normal.vy = v0.vz * v1.vx - v0.vx * v1.vz;
        normal.vz = v0.vx * v1.vy - v0.vy * v1.vx;

        // Normalize the normal vector
        VectorNormal(&normal, &normal);

        // Store the normal in the appropriate place
        m->normals[i / 4].vx = normal.vx;
        m->normals[i / 4].vy = normal.vy;
        m->normals[i / 4].vz = normal.vz;

        printf("Normal for quad %d: (%ld, %ld, %ld)\n", i / 4, normal.vx, normal.vy, normal.vz);
    }
}
