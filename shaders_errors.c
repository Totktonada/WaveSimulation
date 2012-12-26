#include <stdlib.h>
#include <stdio.h>
#include "shaders_errors.h"

/* Based on: see LINKS file: [1]. */
void printInfoLog(FILE * stream, GLuint obj)
{
    int infoLogLength = 0;
    char * infoLog = NULL;
    int charsWritten = 0;

    if (! glIsShader(obj) && ! glIsProgram(obj))
    {
        fprintf(stream,
            "Cannot print InfoLog, \
object is not shader or shader program.");
        return;
    }

    if (glIsShader(obj))
    {
        glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infoLogLength);
    }
    else if (glIsProgram(obj))
    {
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infoLogLength);
    }

    fprintf(stream, "InfoLog length: %d.\n", infoLogLength);
 
    if (infoLogLength <= 0)
    {
        return;
    }

    infoLog = (char *) malloc(infoLogLength * sizeof(char));

    if (glIsShader(obj))
    {
        glGetShaderInfoLog(obj, infoLogLength,
            &charsWritten, infoLog);
    }
    else if (glIsProgram(obj))
    {
        glGetProgramInfoLog(obj, infoLogLength,
            &charsWritten, infoLog);
    }

    fprintf(stream, "%s\n", infoLog);
    free(infoLog);
}

void checkCompileStatus(GLuint obj)
{
    GLint ok;

    glGetShaderiv(obj, GL_COMPILE_STATUS, &ok);

    if (ok == GL_FALSE)
    {
        fprintf(stderr,
            "==== Compile shader or shader program failed ====\n");
        printInfoLog(stderr, obj);
        exit(EXIT_FAILURE);
    }
}

void checkLinkStatus(GLuint obj)
{
    GLint ok;

    glGetProgramiv(obj, GL_LINK_STATUS, &ok);

    if (ok == GL_FALSE)
    {
        fprintf(stderr, "==== Link shader program failed ====\n");
        printInfoLog(stderr, obj);
        exit(EXIT_FAILURE);
    }
}

void validateShaderProgram(GLuint obj)
{
    GLint status;

    glValidateProgram(obj);
    glGetProgramiv(obj, GL_VALIDATE_STATUS, &status);

    if (status == GL_FALSE)
    {
        fprintf(stderr, "Shader program validate failed.\n");
        exit(EXIT_FAILURE);
    }
}
