#include <string.h>
#include <math.h>
#include "matrix.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* pi/2 */
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

void setPerspectiveMatrix(mat4 m,
    float angleY, float aspect, float znear, float zfar)
{
    float f = 1 / tanf((angleY * M_PI / 180) / 2);
    float a = (zfar + znear) / (znear - zfar);
    float b = 2 * zfar * znear / (znear - zfar);

    memset(m, 0, 16 * sizeof(float));

    m[0] = f / aspect;
    m[5] = f;
    m[10] = a;
    m[11] = b;
    m[14] = -1.0f;
}

void setRotationFromHPR(mat4 m, float head, float pitch, float roll)
{
    float cosH = cosf(head  * M_PI / 180.0f);
    float cosP = cosf(pitch * M_PI / 180.0f);
    float cosR = cosf(roll  * M_PI / 180.0f);
    float sinH = sinf(head  * M_PI / 180.0f);
    float sinP = sinf(pitch * M_PI / 180.0f);
    float sinR = sinf(roll  * M_PI / 180.0f);

    m[0] = cosR * cosH - sinR * sinP * sinH;
    m[1] = sinR * cosH + cosR * sinP * sinH;
    m[2] = -cosP * sinH;
    m[3] = 0.0f;

    m[4] = -sinR * cosP;
    m[5] = cosR * cosP;
    m[6] = sinP;
    m[7] = 0.0f;

    m[ 8] = cosR * sinH + sinR * sinP * cosH;
    m[ 9] = sinR * sinH - cosR * sinP * cosH;
    m[10] = cosP * cosH;
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

void setTranslationMatrix(mat4 m, float x, float y, float z)
{
    memset(m, 0, 16 * sizeof(float));

    m[0] = 1.0f;
    m[3] = x;

    m[5] = 1.0f;
    m[7] = y;

    m[10] = 1.0f;
    m[11] = z;

    m[15] = 1.0f;
}

void setMulMatrix(mat4 m, const mat4 a, const mat4 b)
{
    m[ 0] = a[ 0] * b[ 0] + a[ 1] * b[ 4] + a[ 2] * b[ 8] + a[ 3] * b[12];
    m[ 1] = a[ 0] * b[ 1] + a[ 1] * b[ 5] + a[ 2] * b[ 9] + a[ 3] * b[13];
    m[ 2] = a[ 0] * b[ 2] + a[ 1] * b[ 6] + a[ 2] * b[10] + a[ 3] * b[14];
    m[ 3] = a[ 0] * b[ 3] + a[ 1] * b[ 7] + a[ 2] * b[11] + a[ 3] * b[15];

    m[ 4] = a[ 4] * b[ 0] + a[ 5] * b[ 4] + a[ 6] * b[ 8] + a[ 7] * b[12];
    m[ 5] = a[ 4] * b[ 1] + a[ 5] * b[ 5] + a[ 6] * b[ 9] + a[ 7] * b[13];
    m[ 6] = a[ 4] * b[ 2] + a[ 5] * b[ 6] + a[ 6] * b[10] + a[ 7] * b[14];
    m[ 7] = a[ 4] * b[ 3] + a[ 5] * b[ 7] + a[ 6] * b[11] + a[ 7] * b[15];

    m[ 8] = a[ 8] * b[ 0] + a[ 9] * b[ 4] + a[10] * b[ 8] + a[11] * b[12];
    m[ 9] = a[ 8] * b[ 1] + a[ 9] * b[ 5] + a[10] * b[ 9] + a[11] * b[13];
    m[10] = a[ 8] * b[ 2] + a[ 9] * b[ 6] + a[10] * b[10] + a[11] * b[14];
    m[11] = a[ 8] * b[ 3] + a[ 9] * b[ 7] + a[10] * b[11] + a[11] * b[15];

    m[12] = a[12] * b[ 0] + a[13] * b[ 4] + a[14] * b[ 8] + a[15] * b[12];
    m[13] = a[12] * b[ 1] + a[13] * b[ 5] + a[14] * b[ 9] + a[15] * b[13];
    m[14] = a[12] * b[ 2] + a[13] * b[ 6] + a[14] * b[10] + a[15] * b[14];
    m[15] = a[12] * b[ 3] + a[13] * b[ 7] + a[14] * b[11] + a[15] * b[15];
}

void transposeMatrix(mat4 m)
{
    float tmp;

#define SWAP_TRANSPOSE_MATRIX_LOCAL(tmp, a, b) \
    do \
    { \
        (tmp) = (a); \
        (a) = (b); \
        (b) = (tmp); \
    } while(0)

    SWAP_TRANSPOSE_MATRIX_LOCAL(tmp, m[1], m[4]);
    SWAP_TRANSPOSE_MATRIX_LOCAL(tmp, m[2], m[8]);
    SWAP_TRANSPOSE_MATRIX_LOCAL(tmp, m[6], m[9]);
    SWAP_TRANSPOSE_MATRIX_LOCAL(tmp, m[3], m[12]);
    SWAP_TRANSPOSE_MATRIX_LOCAL(tmp, m[7], m[13]);
    SWAP_TRANSPOSE_MATRIX_LOCAL(tmp, m[11], m[14]);

#undef SWAP_TRANSPOSE_MATRIX_LOCAL
}

void setMulMatrixVec3(vec3 v, mat4 m, vec3 a)
{
    v[0] = m[0] * a[0] + m[1] * a[1] + m[ 2] * a[2];
    v[1] = m[4] * a[0] + m[5] * a[1] + m[ 6] * a[2];
    v[2] = m[8] * a[0] + m[9] * a[1] + m[10] * a[2];
}

void setQuaternionFromMatrix(Quaternion q, const mat4 m)
{
    float s = 0.0f;
    float trace = m[0] + m[5] + m[10] + m[15];

    if (trace > 0.00000001f)
    {
        s = sqrtf(trace) * 2.0f;
        q[0] = (m[9] - m[6]) / s;
        q[1] = (m[2] - m[8]) / s;
        q[2] = (m[4] - m[1]) / s;
        q[3] = 0.25f * s;
    }
    else
    {
        int nxt[3] = {1, 2, 0};
        int i = 0, j = 0, k = 0;

        if (m[5] > m[0])
            i = 1;

        if (m[10] > m[5 * i])
            i = 2;

        j = nxt[i];
        k = nxt[j];

        s = 2 * sqrtf(1.0f + m[5 * i] - m[5 * j] - m[5 * k]);

        q[i] = 0.25f * s;
        q[j] = (m[4 * i + j] + m[4 * j + i]) / s;
        q[k] = (m[4 * i + k] + m[4 * k + i]) / s;
        q[3] = (m[4 * k + j] - m[4 * j + k]) / s;
    }
}

void setMatrixFromQuaternion(mat4 m, const Quaternion q)
{
    float x = q[0];
    float y = q[1];
    float z = q[2];
    float w = q[3];

    float x2 = x + x; 
    float y2 = y + y; 
    float z2 = z + z;

    float xx = x * x2;
    float xy = x * y2;
    float xz = x * z2;
    float yy = y * y2;
    float yz = y * z2;
    float zz = z * z2;

    float wx = w * x2;
    float wy = w * y2;
    float wz = w * z2;

    m[ 0] = 1.0f - (yy + zz);
    m[ 1] = xy - wz;
    m[ 2] = xz + wy;
    m[ 3] = 0.0f;

    m[ 4] = xy + wz;
    m[ 5] = 1.0f - (xx + zz);
    m[ 6] = yz - wx;
    m[ 7] = 0.0f;

    m[ 8] = xz - wy;
    m[ 9] = yz + wx;
    m[10] = 1.0f - (xx + yy);
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

void setMulQuaternion(Quaternion q1, const Quaternion q2)
{
    Quaternion t;

    t[0] = q1[0];
    t[1] = q1[1];
    t[2] = q1[2];
    t[3] = q1[3];

    q1[0] = t[3] * q2[0] + t[0] * q2[3] - t[1] * q2[2] + t[2] * q2[1];
    q1[1] = t[3] * q2[1] + t[0] * q2[2] + t[1] * q2[3] - t[2] * q2[0],
    q1[2] = t[3] * q2[2] - t[0] * q2[1] + t[1] * q2[0] + t[2] * q2[3];
    q1[3] = t[3] * q2[3] - t[0] * q2[0] - t[1] * q2[1] - t[2] * q2[2];
}
