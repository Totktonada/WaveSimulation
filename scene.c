#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/glfw.h>
#include "scene.h"
#include "camera.h"
#include "shaders_errors.h"

/* ==== Defines ==== */

#define CONTEXT_WIDTH_INITIAL 640
#define CONTEXT_HEIGHT_INITIAL 480

/* ==== Functions ==== */

void setupOpenGLContext(ContextSize * context)
{
    if (glfwInit() == GL_FALSE)
    {
        fprintf(stderr, "glfwInit failed.\n");
        exit(EXIT_FAILURE);
    }

    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (glfwOpenWindow(
            context->w, context->h,
            0, 0, 0, /* red, green, blue bits -> default */
            0, 0, 0, /* alpha, depth, stencil bits -> default */
            GLFW_WINDOW) == GL_FALSE)
    {
        fprintf(stderr, "glfwOpenWindow failed.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetWindowTitle("Wave Simulation");

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void checkOpenGLVersion()
{
    GLenum glewInitValue;

    /* For compatinility with OpenGL 3.2+ core context. */
    glewExperimental = GL_TRUE;

    glewInitValue = glewInit();

    /* Avoid GLEW errors, see LINKS: [10]. */
    glGetError();

    if (glewInitValue != GLEW_OK)
    {
        fprintf(stderr, "glewInit failed: %s\n.",
            glewGetErrorString(glewInitValue));
        exit(EXIT_FAILURE);
    }

    if (! glewIsSupported("GL_VERSION_3_3"))
    {
        fprintf(stderr, "No OpenGL 3.3 support. I'm sorry.\n");
        fprintf(stderr, "Your OpenGL version: %s.\n",
            glGetString(GL_VERSION));
        exit(EXIT_FAILURE);
    }

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void setupOpenGLState(ContextSize * context, GLboolean vsync)
{
    if (vsync)
    {
        glfwSwapInterval(1);
    }
    else
    {
        glfwSwapInterval(0);
    }

    glViewport(0, 0, context->w, context->h);
    glClearColor(0.0f, 0.20f, 0.40f, 1.0f);

#if 0
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
#endif

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

    /* No interpolate 'flat' varying variables, i.e. normals. */
    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

    /* Perform some rendering with near and far planes clipping off. */
    /* I use small znear value instead, that work better. */
    /* glEnable(GL_DEPTH_CLAMP); */

    /* For correct bilinear interpolation at rib of skybox. */
    /* TODO: glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);*/

#if 0
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
#endif

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

Scene * newScene(GLboolean vsync)
{
    Scene * scene = (Scene *) malloc(sizeof(Scene));

    scene->context = (ContextSize *) malloc(sizeof(ContextSize));
    scene->context->w = CONTEXT_WIDTH_INITIAL;
    scene->context->h = CONTEXT_HEIGHT_INITIAL;

    setupOpenGLContext(scene->context);
    checkOpenGLVersion();
    setupOpenGLState(scene->context, vsync);

    scene->camera = newCamera(scene->context->w, scene->context->h);

    scene->world = getWorld("world.txt");
    scene->water = getWater();
    setupWater(scene->water->drawSP, scene->world);
    setupWorldUniforms(scene->world->sp, scene->water);

    return scene;
}

void freeScene(Scene * scene)
{
    freeCamera(scene->camera);
    freeWorld(scene->world);
    freeWater(scene->water);
    free(scene->context);
    free(scene);
    glfwTerminate();
}
