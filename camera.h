#ifndef CAMERA_H_SENTRY
#define CAMERA_H_SENTRY

#include <GL/glew.h>
#include "matrix.h"
#include "shaders.h"

typedef
struct Camera
{
    /* ---- Position ---- */
    vec3 pos;

    /* ---- Look direction ---- */
    Quaternion q;

    /* ---- Projection ---- */
    float viewAngleY;
    float aspect;
    GLclampd znear;
    GLclampd zfar;
}
Camera;

Camera * newCamera(int w, int h);

/* r, u, f -- right, up, forward. */
void slideCamera(Camera * camera, float dr, float du, float df);

void rotateCamera(Camera * camera, float head, float pitch, float roll);

void setupCamera(ShaderProgram * sp, const Camera * camera);

void setupCameraAlter(ShaderProgram * sp, const Camera * camera);

void freeCamera(Camera * camera);

#endif /* CAMERA_H_SENTRY */
