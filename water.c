#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "water.h"
#include "texture.h"
#include "shaders_errors.h"

void checkFramebufferStatus()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "glCheckFramebufferStatus returned %d.\n", status);
        exit(EXIT_FAILURE);
    }
}

void initWaterMeshData(Water * water)
{
    water->data = (MeshData *) malloc(sizeof(MeshData));

    water->data->w = 64;
    water->data->h = 64;
    water->data->firstX = -10.0f;
    water->data->lastX = 10.0f;
    water->data->firstY = -10.0f;
    water->data->lastY = 10.0f;
    water->data->z = 0.50f;
}

void genTextures(Water * water)
{
    int w = water->data->w;
    int h = water->data->h;
    GLfloat z = water->data->z;

    int x, y;

    GLfloat * data = (GLfloat *) malloc(w * h * 4 * sizeof(GLfloat));

    int base = 0;

    for (y = 0; y < h; ++y)
    {
        for (x = 0; x < w; ++x)
        {
            data[base + 0] = z;
            data[base + 1] = 0.0f;
            data[base + 2] = 0.0f;
            data[base + 3] = 0.0f;

            base += 4;
        }
    }

    water->textureIds = (GLuint *) malloc(3 * sizeof(GLuint));

    water->textureIds[0] = createTexture(w, h, GL_RGBA32F, data);
    water->textureIds[1] = createTexture(w, h, GL_RGBA32F, data);
    water->textureIds[2] = createTexture(w, h, GL_RGBA32F, data);

    free(data);
}

void rotateTextures(Water * water)
{
    int nxt[] = {1, 2, 0};

    GLuint newFst = nxt[water->firstTexture];
    GLuint newSnd = nxt[newFst];
    GLuint newDst = nxt[newSnd];

    water->firstTexture = newFst;

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, water->textureIds[newFst]);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, water->textureIds[newSnd]);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, water->textureIds[newDst]);

    glBindFramebuffer(GL_FRAMEBUFFER, water->fboId);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, water->textureIds[newDst], 0);

#ifdef DEBUG
    checkFramebufferStatus();
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void initModifyWaterShaderProgram(Water * water, const GLfloat * mesh,
    const GLuint * idx)
{
    water->modifySP = getShaderProgram("modify_water_vshader.glsl",
        NULL, "modify_water_fshader.glsl");

    glGenVertexArrays(1, &(water->modifyVaoP));
    glBindVertexArray(water->modifyVaoP);

    setupVbo(water->modifySP, mesh, "position", 3,
        water->data->w * water->data->h);
    setupIdxVbo(water->modifySP, idx, water->idxCnt);

    glGenFramebuffers(1, &(water->fboId));

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void initDrawWaterShaderProgram(Water * water, const GLfloat * mesh,
    const GLuint * idx)
{
    water->drawSP = getShaderProgram("draw_water_vshader.glsl",
        NULL, "draw_water_fshader.glsl");

    glGenVertexArrays(1, &(water->drawVaoP));
    glBindVertexArray(water->drawVaoP);

    setupVbo(water->drawSP, mesh, "position", 3,
        water->data->w * water->data->h);
    setupIdxVbo(water->drawSP, idx, water->idxCnt);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void setupMeshUniforms(Water * water)
{
    int w = water->data->w;
    int h = water->data->h;

    float sizeX = water->data->lastX - water->data->firstX;
    float sizeY = water->data->lastY - water->data->firstY;

    GLuint obj;

    glUseProgram(water->modifySP->p);

    obj = glGetUniformLocation(water->modifySP->p, "meshTexStep");
    /* TODO: if (obj == -1) {} */
    glUniform2f(obj, 1.0f / (w - 1), 1.0f / (h - 1));

    obj = glGetUniformLocation(water->modifySP->p, "meshViewFirst");
    /* TODO: if (obj == -1) {} */
    glUniform2f(obj, water->data->firstX, water->data->firstY);

    obj = glGetUniformLocation(water->modifySP->p, "meshViewSize");
    /* TODO: if (obj == -1) {} */
    glUniform2f(obj, sizeX, sizeY);

    obj = glGetUniformLocation(water->modifySP->p, "meshZ");
    /* TODO: if (obj == -1) {} */
    glUniform1f(obj, water->data->z);

    obj = glGetUniformLocation(water->modifySP->p, "makeWave");
    /* TODO: if (obj == -1) {} */
    glUniform1i(obj, GL_FALSE);

    glUseProgram(water->drawSP->p);

    obj = glGetUniformLocation(water->drawSP->p, "meshTexStep");
    /* TODO: if (obj == -1) {} */
    glUniform2f(obj, 1.0f / (w - 1), 1.0f / (h - 1));

    obj = glGetUniformLocation(water->drawSP->p, "meshViewFirst");
    /* TODO: if (obj == -1) {} */
    glUniform2f(obj, water->data->firstX, water->data->firstY);

    obj = glGetUniformLocation(water->drawSP->p, "meshViewSize");
    /* TODO: if (obj == -1) {} */
    glUniform2f(obj, sizeX, sizeY);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
    glUseProgram(0);
}

void setupWorldUniforms(ShaderProgram * sp, Water * water)
{
    float sizeX = water->data->lastX - water->data->firstX;
    float sizeY = water->data->lastY - water->data->firstY;

    GLuint obj;

    glUseProgram(sp->p);

    obj = glGetUniformLocation(sp->p, "texGeometry");
    /* TODO: if (obj == -1) {} */
    glUniform1i(obj, 4);

    obj = glGetUniformLocation(sp->p, "meshViewFirst");
    /* TODO: if (obj == -1) {} */
    glUniform2f(obj, water->data->firstX, water->data->firstY);

    obj = glGetUniformLocation(sp->p, "meshViewSize");
    /* TODO: if (obj == -1) {} */
    glUniform2f(obj, sizeX, sizeY);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
    glUseProgram(0);
}

void setupTextureUniforms(Water * water)
{
    GLuint obj;

    glUseProgram(water->modifySP->p);

    obj = glGetUniformLocation(water->modifySP->p, "texFst");
    /* TODO: if (obj == -1) {} */
    glUniform1i(obj, 2);

    obj = glGetUniformLocation(water->modifySP->p, "texSnd");
    /* TODO: if (obj == -1) {} */
    glUniform1i(obj, 3);

    glUseProgram(water->drawSP->p);

    obj = glGetUniformLocation(water->drawSP->p, "texGeometry");
    /* TODO: if (obj == -1) {} */
    glUniform1i(obj, 4);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

Water * getWater()
{
    Water * water = (Water *) malloc(sizeof(Water));

    GLfloat * mesh;
    GLuint * idx;

    initWaterMeshData(water);

    mesh = meshGenVertices(water->data);
    water->idxCnt = meshGenIdx(water->data, &idx);

    initModifyWaterShaderProgram(water, mesh, idx);
    initDrawWaterShaderProgram(water, mesh, idx);

    setupMeshUniforms(water);

    genTextures(water);

    setupTextureUniforms(water);

    water->firstTexture = 2;
    rotateTextures(water);

    free(mesh);
    free(idx);

    water->dSecondSum = 0.0f;

    return water;
}

typedef
struct RenderState
{
    GLint viewport[4];
    GLboolean depthTest;
}
RenderState;

void saveRenderState(RenderState * state)
{
    glGetIntegerv(GL_VIEWPORT, state->viewport);
    state->depthTest = glIsEnabled(GL_DEPTH_TEST);
}

void restoreRenderState(const RenderState * state)
{
    glViewport(state->viewport[0], state->viewport[1],
        state->viewport[2], state->viewport[3]);

    if (state->depthTest)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
}

void modifyWaterMeshStep(Water * water)
{
    RenderState state;

    saveRenderState(&state);

    glUseProgram(water->modifySP->p);
    glBindVertexArray(water->modifyVaoP);

    glBindFramebuffer(GL_FRAMEBUFFER, water->fboId);

    glViewport(0, 0, water->data->w, water->data->h);
    glDisable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, water->idxCnt,
        GL_UNSIGNED_INT, NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    restoreRenderState(&state);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void modifyWaterMesh(Water * water, float dSecond)
{
    GLuint obj;

    water->dSecondSum += dSecond;

    while (water->dSecondSum > 0.05f)
    {
        rotateTextures(water);
        modifyWaterMeshStep(water);
        water->dSecondSum -= 0.05f;

        obj = glGetUniformLocation(water->modifySP->p, "makeWave");
        /* TODO: if (obj == -1) {} */
        glUniform1i(obj, GL_FALSE);

        CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
    }
}

void setWaterWave(Water * water)
{
    GLuint obj;

    glUseProgram(water->modifySP->p);

    obj = glGetUniformLocation(water->modifySP->p, "makeWave");
    /* TODO: if (obj == -1) {} */
    glUniform1i(obj, GL_TRUE);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void drawWater(const Water * water)
{
    glUseProgram(water->drawSP->p);
    glBindVertexArray(water->drawVaoP);

    glDrawElements(GL_TRIANGLES, water->idxCnt,
        GL_UNSIGNED_INT, NULL);
}

void freeWater(Water * water)
{
    free(water->data);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteTextures(1, &(water->textureIds[0]));
    glDeleteTextures(1, &(water->textureIds[1]));
    glDeleteTextures(1, &(water->textureIds[2]));

    free(water->textureIds);

    glDeleteFramebuffers(1, &(water->fboId));
}
