#ifndef WORLD_H_SENTRY
#define WORLD_H_SENTRY

#include "matrix.h"
#include "shaders.h"
#include <GL/glfw.h>

typedef
struct PointLight
{
    vec3 position;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    vec3 attenuation;
}
PointLight;

typedef
struct Texture
{
    struct Texture * next;

    const char * name;
    GLuint id;
    int num;
}
Texture;

typedef
struct TextureList
{
    Texture * first;
    Texture * last;
    int cnt;
}
TextureList;

typedef
struct Material
{
    struct Material * next;

    char * name;

    char * textureName;

    vec4 emission;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    GLfloat shininess;
}
Material;

typedef
struct MaterialList
{
    Material * first;
    Material * last;
}
MaterialList;

typedef
struct WorldObject
{
    struct WorldObject * next;

    const char * materialName;
    GLenum primitiveType; /* I.e. GL_TRIANGLES. */

    GLfloat * position;
    GLfloat * normal;
    GLfloat * texCoord;
    GLsizei cnt;

    GLuint * idx;
    GLsizei idxCnt;

    GLuint vaoP;
}
WorldObject;

typedef
struct WorldObjectList
{
    WorldObject * first;
    WorldObject * last;
}
WorldObjectList;

typedef
struct World
{
    PointLight * pointLight;
    TextureList texList;
    MaterialList mtrlList;
    WorldObjectList objList;

    ShaderProgram * sp;
}
World;

World * getWorld(const char * path);

void drawWorld(World * world);

void setupWater(ShaderProgram * sp, World * world);

void freeWorld(World * world);

#endif /* WORLD_H_SENTRY */
