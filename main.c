#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "main.h"
#include "scene.h"
#include "shaders_errors.h"
#include "shaders.h"
#include "camera.h"
#include "utils.h"
#include <GL/glfw.h>

#define SLIDE_STEP 0.05f
#define ROTATE_STEP 1.0f
#define BASE_FPS 60.0f

typedef
struct Flags
{
    GLboolean pause;
    GLboolean vsync;
}
Flags;

/* ==== Global variables ==== */

/* If anyone known way to avoid use mutable global variables
 * (in this project), then e-mail to me, please. */

static Scene * scene;

/* Is application running (GL_TRUE) or must be terminated (GL_FALSE). */
static int running = GL_TRUE;

static Flags flags;

/* ==== Callbacks ==== */

void GLFWCALL windowResizeCallback(int w, int h)
{
    scene->context->w = w;
    scene->context->h = h;

    scene->camera->aspect = ((float) scene->context->w) /
        ((float) scene->context->h);

    if (scene->world != NULL)
    {
        setupCamera(scene->world->sp, scene->camera);
    }

    if (scene->water != NULL)
    {
        setupCameraAlter(scene->water->modifySP, scene->camera);
        setupCamera(scene->water->drawSP, scene->camera);
    }

    glViewport(0, 0, w, h);
}

void GLFWCALL keyboardCallback(int key, int action)
{
    if (key == GLFW_KEY_PAUSE && action == GLFW_PRESS)
    {
        flags.pause = !flags.pause;
    }

    if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
    {
        flags.vsync = !flags.vsync;

        if (flags.vsync)
        {
            glfwSwapInterval(1);
        }
        else
        {
            glfwSwapInterval(0);
        }
    }
}

void GLFWCALL mouseButtonCallback(int button, int action)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS &&
        !flags.pause)
    {
        setWaterWave(scene->water);
    }
}

void processKeyboardEvents(float factor)
{
    int cameraModified = 0;
    float slide = SLIDE_STEP * factor;
    float rotate = ROTATE_STEP * factor;

    if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS)
    {
        running = GL_FALSE;
    }

    if (glfwGetKey('W') == GLFW_PRESS)
    {
        slideCamera(scene->camera, 0.0f, 0.0f, -slide);
        cameraModified = 1;
    }

    if (glfwGetKey('S') == GLFW_PRESS)
    {
        slideCamera(scene->camera, 0.0f, 0.0f, slide);
        cameraModified = 1;
    }

    if (glfwGetKey('A') == GLFW_PRESS)
    {
        slideCamera(scene->camera, -slide, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey('D') == GLFW_PRESS)
    {
        slideCamera(scene->camera, slide, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(GLFW_KEY_PAGEUP) == GLFW_PRESS)
    {
        slideCamera(scene->camera, 0.0f, slide, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(GLFW_KEY_PAGEDOWN) == GLFW_PRESS)
    {
        slideCamera(scene->camera, 0.0f, -slide, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS)
    {
        rotateCamera(scene->camera, 0.0f, -rotate, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        rotateCamera(scene->camera, 0.0f, rotate, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        rotateCamera(scene->camera, rotate, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        rotateCamera(scene->camera, -rotate, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey('Q') == GLFW_PRESS)
    {
        rotateCamera(scene->camera, 0.0, 0.0f, rotate);
        cameraModified = 1;
    }

    if (glfwGetKey('E') == GLFW_PRESS)
    {
        rotateCamera(scene->camera, 0.0f, 0.0f, -rotate);
        cameraModified = 1;
    }

    if (cameraModified && scene->world != NULL)
    {
        setupCamera(scene->world->sp, scene->camera);
    }

    if (cameraModified && scene->water != NULL)
    {
        setupCameraAlter(scene->water->modifySP, scene->camera);
        setupCamera(scene->water->drawSP, scene->camera);
    }
}

void processMouseEvents(float factor)
{
    int x;
    int y;

    int cx = scene->context->w / 2;
    int cy = scene->context->h / 2;

    float sensitivity = 0.05f;

    float dx, dy;

    glfwGetMousePos(&x, &y);

    dx = sensitivity * ((float) cx - (float) x);
    dy = sensitivity * ((float) cy - (float) y);

    if (dx == 0.0f && dy == 0.0f)
    {
        return;
    }

    if (glfwGetKey(GLFW_KEY_LSHIFT) == GLFW_PRESS ||
        glfwGetKey(GLFW_KEY_RSHIFT) == GLFW_PRESS)
    {
        float slide = SLIDE_STEP * factor;

        dx *= slide;
        dy *= slide;

        slideCamera(scene->camera, -dx, dy, 0.0f);
    }
    else
    {
        float rotate = ROTATE_STEP * factor;

        dx *= rotate;
        dy *= rotate;

        rotateCamera(scene->camera, dx, 0.0f, 0.0f);
        rotateCamera(scene->camera, 0.0f, -dy, 0.0f);
    }

    if (scene->world != NULL)
    {
        setupCamera(scene->world->sp, scene->camera);
    }

    if (scene->water != NULL)
    {
        setupCameraAlter(scene->water->modifySP, scene->camera);
        setupCamera(scene->water->drawSP, scene->camera);
    }

    glfwSetMousePos(cx, cy);
}

void setupGLFWCallbacks()
{
    glfwSetWindowSizeCallback(windowResizeCallback);

    glfwEnable(GLFW_STICKY_KEYS);
    glfwSetKeyCallback(keyboardCallback);

    glfwEnable(GLFW_STICKY_MOUSE_BUTTONS);
    glfwSetMouseButtonCallback(mouseButtonCallback);
}

void draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawWorld(scene->world);
    drawWater(scene->water);
}

void viewFps(int frameCnt, float diffSum)
{
    static char title[64];
    float fps = frameCnt / diffSum;

    if (flags.vsync && flags.pause)
    {
        sprintf(title, "Wave Simulation; FPS: %0.0f [vsync, paused]", fps);
    }
    else if (flags.vsync)
    {
        sprintf(title, "Wave Simulation; FPS: %0.0f [vsync]", fps);
    }
    else if (flags.pause)
    {
        sprintf(title, "Wave Simulation; FPS: %0.0f [paused]", fps);
    }
    else
    {
        sprintf(title, "Wave Simulation; FPS: %0.0f", fps);
    }

    glfwSetWindowTitle(title);
}

/* ==== Main ==== */

int main()
{
    int frameCnt = 0;
    struct timeval curTime;
    float dSecond = 0;
    float dSecondSum = 0; /* droped if more than 0.5 sec */

    timeval_diff_replace(&curTime);

    flags.pause = GL_FALSE;
    flags.vsync = GL_TRUE;

    scene = newScene(flags.vsync);

    setupGLFWCallbacks();
    glfwSetMousePos(scene->context->w / 2, scene->context->h / 2);

    while (running)
    {
        if (glfwGetWindowParam(GLFW_OPENED) == GL_FALSE)
        {
            running = GL_FALSE;
            break;
        }

        processKeyboardEvents(dSecond * BASE_FPS);
        processMouseEvents(dSecond * BASE_FPS);

        ++frameCnt;
        dSecond = timeval_diff_replace(&curTime);
        dSecondSum += dSecond;

        if (dSecondSum > 0.5f)
        {
            viewFps(frameCnt, dSecondSum);
            frameCnt = 0;
            dSecondSum = 0.0f;
        }

        if (!flags.pause)
        {
            modifyWaterMesh(scene->water, dSecond);
        }

        draw();
        glfwSwapBuffers();
    }

    freeScene(scene);

    return EXIT_SUCCESS;
}
