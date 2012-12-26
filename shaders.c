#include <stdlib.h>
#include <stdio.h>
#include "shaders.h"
#include "utils.h"
#include "shaders_errors.h"

ShaderProgram * getShaderProgram(const char * vPath,
    const char * gPath, const char * fPath)
{
    ShaderProgram * sp = (ShaderProgram *) malloc(sizeof(ShaderProgram));

    char * vSrc = getTextFileContent(vPath, NULL);
    char * gSrc = (gPath != NULL) ?
        getTextFileContent(gPath, NULL) : NULL;
    char * fSrc = getTextFileContent(fPath, NULL);

    if (vSrc == NULL || fSrc == NULL ||
        (gPath != NULL && gSrc == NULL))
    {
        fprintf(stderr,
           "getShaderProgram() failed: file can not be read.\n");
        exit(EXIT_FAILURE);
    }

    sp->v = glCreateShader(GL_VERTEX_SHADER);
    sp->g = (gPath != NULL) ? glCreateShader(GL_GEOMETRY_SHADER) : 0;
    sp->f = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(sp->v, 1, (const char **) &vSrc, NULL);
    glShaderSource(sp->f, 1, (const char **) &fSrc, NULL);

    glCompileShader(sp->v);
    checkCompileStatus(sp->v);
    glCompileShader(sp->f);
    checkCompileStatus(sp->f);

    free(vSrc);
    free(fSrc);

    sp->p = glCreateProgram();

    glAttachShader(sp->p, sp->v);
    glAttachShader(sp->p, sp->f);

    if (gPath != NULL)
    {
        glShaderSource(sp->g, 1, (const char **) &gSrc, NULL);
        glCompileShader(sp->g);
        checkCompileStatus(sp->g);
        free(gSrc);
        glAttachShader(sp->p, sp->g);
    }

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);

    glLinkProgram(sp->p);
    checkLinkStatus(sp->p);

    glUseProgram(sp->p);

    glValidateProgram(sp->p);

    return sp;
}

void freeShaderProgram(ShaderProgram * sp)
{
    glUseProgram(0);

    glDetachShader(sp->p, sp->v);
    if (sp->g != 0)
    {
        glDetachShader(sp->p, sp->g);
    }
    glDetachShader(sp->p, sp->f);

    glDeleteShader(sp->v);
    if (sp->g != 0)
    {
        glDeleteShader(sp->g);
    }
    glDeleteShader(sp->f);

    glDeleteProgram(sp->p);
}

void setupVbo(ShaderProgram * sp, const GLfloat * data,
    const char * attrName, int groupSize, GLsizei cnt)
{
    GLuint vboP, attribP;

    glUseProgram(sp->p);

    glGenBuffers(1, &vboP);
    glBindBuffer(GL_ARRAY_BUFFER, vboP);
    glBufferData(GL_ARRAY_BUFFER, cnt * groupSize * sizeof(GLfloat), data,
        GL_STATIC_DRAW);

    attribP = glGetAttribLocation(sp->p, attrName);
    /* TODO: if (attribP == -1) {} */
    glVertexAttribPointer(attribP, groupSize, GL_FLOAT, GL_FALSE, 0,
        (const GLvoid *) 0);
    glEnableVertexAttribArray(attribP);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void setupIdxVbo(ShaderProgram * sp, const GLuint * idx, GLsizei cnt)
{
    GLuint vboIdxP;

    glUseProgram(sp->p);

    glGenBuffers(1, &vboIdxP);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIdxP);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cnt * sizeof(GLuint),
        idx, GL_STATIC_DRAW);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}
