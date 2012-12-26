#include <stdlib.h>
#include <GL/glew.h>
#include "mesh.h"

/* w, h - count of vertices in horizontal/vertical line. */
GLfloat * meshGenVertices(const MeshData * data)
{
    GLfloat stepX = (data->lastX - data->firstX) / (data->w - 1);
    GLfloat stepY = (data->lastY - data->firstY) / (data->h - 1);

    int w = data->w;
    int h = data->h;
    GLfloat z = data->z;

    int x, y;

    GLfloat * mesh = (GLfloat *) malloc(3 * w * h * sizeof(GLfloat));

    GLsizei base = 0;

    for (y = 0; y < h; ++y)
    {
        for (x = 0; x < w; ++x)
        {
            mesh[base + 0] = data->firstX + x * stepX;
            mesh[base + 1] = data->firstY + y * stepY;
            mesh[base + 2] = z;

            base += 3;
        }
    }

    return mesh;
}

GLsizei meshGenIdx(const MeshData * data, GLuint ** idxP)
{
    int w = data->w;
    int h = data->h;

    GLuint * idx =
        (GLuint *) malloc(6 * (w - 1) * (h - 1) * sizeof(GLuint));

    int x, y;
    GLsizei base = 0;

    for (y = 0; y < h - 1; ++y)
    {
        for (x = 0; x < w - 1; ++x)
        {
            idx[base + 0] = (y + 0) * w + (x + 0);
            idx[base + 1] = (y + 0) * w + (x + 1);
            idx[base + 2] = (y + 1) * w + (x + 0);

            idx[base + 3] = idx[base + 1];
            idx[base + 4] = (y + 1) * w + (x + 1);
            idx[base + 5] = idx[base + 2];

            base += 6;
        }
    }

    *idxP = idx;

    return base;
}


