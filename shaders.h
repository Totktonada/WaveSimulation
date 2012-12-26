#ifndef SHADERS_H_SENTRY
#define SHADERS_H_SENTRY

#include <GL/glew.h>

typedef
struct ShaderProgram
{
    GLuint v; /* vertex shader pointer */
    GLuint g; /* geometry shader pointer */
    GLuint f; /* fragment shader pointer */
    GLuint p; /* shader program pointer */
}
ShaderProgram;

ShaderProgram * getShaderProgram(const char * vPath,
    const char * gPath, const char * fPath);

void freeShaderProgram(ShaderProgram * sp);

void setupVbo(ShaderProgram * sp, const GLfloat * data,
    const char * attrName, int groupSize, GLsizei cnt);

void setupIdxVbo(ShaderProgram * sp, const GLuint * idx, GLsizei cnt);

#endif /* SHADERS_H_SENTRY */
