#ifndef WATER_H_SENTRY
#define WATER_H_SENTRY

#include "shaders.h"
#include "mesh.h"

typedef
struct Water
{
    MeshData * data;

    ShaderProgram * modifySP;
    ShaderProgram * drawSP;

    GLuint modifyVaoP;
    GLuint drawVaoP;

    GLuint fboId;

    int firstTexture;
    GLuint * textureIds;

    /* For gen and for draw data */
    GLsizei idxCnt;

    float dSecondSum;
}
Water;

Water * getWater();

void modifyWaterMesh(Water * water, float dSecond);

void setWaterWave(Water * water);

void drawWater(const Water * water);

void freeWater(Water * water);

void setupWorldUniforms(ShaderProgram * sp, Water * water);

#endif /* WATER_H_SENTRY */
