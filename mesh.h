#ifndef MESH_H_SENTRY
#define MESH_H_SENTRY

typedef
struct MeshData
{
    int w;
    int h;
    GLfloat firstX;
    GLfloat lastX;
    GLfloat firstY;
    GLfloat lastY;
    GLfloat z;
}
MeshData;

/* w, h - count of vertices in horizontal/vertical line. */
GLfloat * meshGenVertices(const MeshData * data);

GLsizei meshGenIdx(const MeshData * data, GLuint ** idxP);

#endif /* MESH_H_SENTRY */
