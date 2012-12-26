#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include "texture.h"
#include "utils.h"
#include "shaders_errors.h"

/* Based on: see LINKS: [3]. */

typedef
struct TgaHeader
{
    int idlength:8;
    int colormap:8;
    int datatype:8;
    int colormapinfo_1:8;
    int colormapinfo_2:8;
    int colormapinfo_3:8;
    int colormapinfo_4:8;
    int colormapinfo_5:8;
    int xorigin:16;
    int yorigin:16;
    int width:16;
    int height:16;
    int bitperpel:8;
    int description:8;
} __attribute__((packed))
TgaHeader;

GLuint createTexture(int w, int h, GLint format, const GLvoid * data)
{
    GLuint textureId;

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0,
        GL_RGBA, GL_FLOAT, data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureId;
}

GLuint createTextureFromTga(const char * path)
{
    long length;
    char * buffer = getTextFileContent(path, &length);
    const TgaHeader * header = (TgaHeader *) buffer;

    GLint format;
    GLint internalFormat;
    GLuint texture;
    const GLvoid * imgData;

    if (buffer == NULL)
    {
        return 0;
    }

    if (length <= (signed long) sizeof(TgaHeader))
    {
         fprintf(stderr, "Too small TGA file: %s\n", path);
         free(buffer);
         return 0;
    }

    if (header->datatype != 2 ||
        (header->bitperpel != 24 && header->bitperpel != 32))
    {
         fprintf(stderr, "Wrong TGA file format: %s\n", path);
         free(buffer);
         return 0;
    }

    format = (header->bitperpel == 24 ? GL_BGR : GL_BGRA);
    internalFormat = (format == GL_BGR ? GL_RGB8 : GL_RGBA8);
    imgData = (const GLvoid *)
        (buffer + sizeof(TgaHeader) + header->idlength);

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    /* Set 1-byte alignment (for non (2^n)x(2^n) size textures). */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
        header->width, header->height, 0, format,
        GL_UNSIGNED_BYTE, imgData);

    free(buffer);

#if 0
    /* Linear filtering */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);*/
#else
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif

#if 0
    /* No wrap */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);

    return texture;
}

void freeTexture(GLuint texture)
{
    glDeleteTextures(1, &texture);
}
