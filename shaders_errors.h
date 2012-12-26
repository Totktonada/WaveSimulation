#ifndef SHADERS_ERRORS_H_SENTRY
#define SHADERS_ERRORS_H_SENTRY

#include <GL/glew.h>

#ifdef DEBUG

#define CHECK_OPENGL_ERRORS(file, line) \
    do { \
        GLuint error = glGetError(); \
        if (error != GL_NO_ERROR) \
        { \
            fprintf(stderr, "OpenGL error 0x%X; %s:%d.\n", \
                (unsigned int) error, file, line); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

#else

#define CHECK_OPENGL_ERRORS(file, line) \
    do { \
    } while (0)

#endif

void checkCompileStatus(GLuint obj);

void checkLinkStatus(GLuint obj);

void validateShaderProgram();

#endif /* SHADERS_ERRORS_H_SENTRY */
