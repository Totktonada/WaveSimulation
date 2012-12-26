#ifndef MATRIX_H_SENTRY
#define MATRIX_H_SENTRY

/* Based on: see LINKS file: [2], [3]. */

typedef float mat4[16];
typedef float vec3[3];
typedef float vec4[4];
typedef float Quaternion[4];

void setPerspectiveMatrix(mat4 m,
    float angleY, float aspect, float znear, float zfar);

/* - - - - *
 | 1 0 0 x |
 | 0 1 0 y |
 | 0 0 1 z |
 | 0 0 0 1 |
 * - - - - */
void setTranslationMatrix(mat4 m, float x, float y, float z);

void setMulMatrix(mat4 m, const mat4 a, const mat4 b);

void transposeMatrix(mat4 m);

void setMulMatrixVec3(vec3 v, mat4 m, vec3 a);

void setRotationFromHPR(mat4 m, float head, float pitch, float roll);

void setQuaternionFromMatrix(Quaternion q, const mat4 m);

void setMatrixFromQuaternion(mat4 m, const Quaternion q);

/* Multiply so that rotations are applied in a left to right order. */
void setMulQuaternion(Quaternion q1, const Quaternion q2);

#endif /* MATRIX_H_SENTRY */
