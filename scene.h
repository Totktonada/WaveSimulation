#ifndef SCENE_H_SENTRY
#define SCENE_H_SENTRY

#include "camera.h"
#include "world.h"
#include "water.h"

typedef
struct ContextSize
{
    int w;
    int h;
}
ContextSize;

typedef
struct Scene
{
    ContextSize * context;
    Camera * camera;

    World * world;
    Water * water;
}
Scene;

Scene * newScene(GLboolean vsync);

void freeScene();

#endif /* SCENE_H_SENTRY */
