#include <stdlib.h>
#include <stdio.h>
#include "camera.h"
#include "shaders_errors.h"

Camera * newCamera(int w, int h)
{
    Camera * camera = (Camera *) malloc(sizeof(Camera));

    camera->pos[0] = 18.0f;
    camera->pos[1] = 10.0f;
    camera->pos[2] = 10.0f;

    camera->q[0] = 1.0f;
    camera->q[1] = 0.0f;
    camera->q[2] = 0.0f;
    camera->q[3] = 0.0f;

    rotateCamera(camera, -30.0f, -160.0f, 30.0f);

    camera->viewAngleY = 45.0f;
    camera->aspect = ((float) w) / ((float) h);
    camera->znear = 0.001f;
    camera->zfar = 100.0f;

    return camera;
}

void slideCamera(Camera * camera, float dr, float du, float df)
{
    vec3 xyz;
    vec3 ruf;
    mat4 rot;

    ruf[0] = dr;
    ruf[1] = du;
    ruf[2] = df;

    setMatrixFromQuaternion(rot, camera->q);

    transposeMatrix(rot);

    setMulMatrixVec3(xyz, rot, ruf);

    camera->pos[0] += xyz[0];
    camera->pos[1] += xyz[1];
    camera->pos[2] += xyz[2];
}

void rotateCamera(Camera * camera, float head, float pitch, float roll)
{
    mat4 rot;
    Quaternion dq;

    setRotationFromHPR(rot, head, pitch, roll);
    setQuaternionFromMatrix(dq, rot);
    setMulQuaternion(camera->q, dq);
}

void setupCamera(ShaderProgram * sp, const Camera * camera)
{
    /* Pointers */
    GLint viewProjP, viewPosP;

    /* Matrices */
    mat4 viewTrMatrix, viewRotMatrix, viewMatrix;
    mat4 projMatrix, viewProjMatrix;

    setPerspectiveMatrix(projMatrix,
        camera->viewAngleY,
        camera->aspect,
        camera->znear,
        camera->zfar);

    setMatrixFromQuaternion(viewRotMatrix, camera->q);

    setTranslationMatrix(viewTrMatrix,
        -camera->pos[0],
        -camera->pos[1],
        -camera->pos[2]);

    setMulMatrix(viewMatrix, viewRotMatrix, viewTrMatrix);

    setMulMatrix(viewProjMatrix, projMatrix, viewMatrix);

    glUseProgram(sp->p);

    viewProjP = glGetUniformLocation(sp->p, "transform.viewProjection");
    /* TODO: if (viewProjP == -1) {} */
    glUniformMatrix4fv(viewProjP, 1, GL_TRUE, viewProjMatrix);

    viewPosP = glGetUniformLocation(sp->p, "transform.viewPosition");
    /* TODO: if (viewPosP == -1) {} */
    glUniform3fv(viewPosP, 1, camera->pos);

#ifdef DEBUG
    validateShaderProgram(sp->p);
#endif
    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void setupCameraAlter(ShaderProgram * sp, const Camera * camera)
{
    /* Pointers */
    GLint rotP, viewPosP;

    /* Matrices */
    mat4 rotMatrix;

    setMatrixFromQuaternion(rotMatrix, camera->q);
    transposeMatrix(rotMatrix);

    glUseProgram(sp->p);

    rotP = glGetUniformLocation(sp->p, "transform.rot");
    /* TODO: if (rotP == -1) {} */
    glUniformMatrix4fv(rotP, 1, GL_TRUE, rotMatrix);

    viewPosP = glGetUniformLocation(sp->p, "transform.viewPosition");
    /* TODO: if (viewPosP == -1) {} */
    glUniform3fv(viewPosP, 1, camera->pos);

#ifdef DEBUG
    validateShaderProgram(sp->p);
#endif
    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void freeCamera(Camera * camera)
{
    free(camera);
}
