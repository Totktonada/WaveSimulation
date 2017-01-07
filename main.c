#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "main.h"
#include "scene.h"
#include "shaders_errors.h"
#include "shaders.h"
#include "camera.h"
#include "utils.h"
#include <GLFW/glfw3.h>

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

void windowResizeCallback(GLFWwindow * window, int w, int h)
{
    UNUSED(window);

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

void keyboardCallback(GLFWwindow * window, int key, int scancode,
    int action, int mods)
{
    UNUSED(window);
    UNUSED(scancode);
    UNUSED(mods);

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

void mouseButtonCallback(GLFWwindow * window, int button, int action,
    int mods)
{
    UNUSED(window);
    UNUSED(mods);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS &&
        !flags.pause)
    {
        setWaterWave(scene->water);
    }
}

void processKeyboardEvents(GLFWwindow * window, float factor)
{
    int cameraModified = 0;
    float slide = SLIDE_STEP * factor;
    float rotate = ROTATE_STEP * factor;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        running = GL_FALSE;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        slideCamera(scene->camera, 0.0f, 0.0f, -slide);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        slideCamera(scene->camera, 0.0f, 0.0f, slide);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        slideCamera(scene->camera, -slide, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        slideCamera(scene->camera, slide, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
    {
        slideCamera(scene->camera, 0.0f, slide, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
    {
        slideCamera(scene->camera, 0.0f, -slide, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        rotateCamera(scene->camera, 0.0f, -rotate, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        rotateCamera(scene->camera, 0.0f, rotate, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        rotateCamera(scene->camera, rotate, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        rotateCamera(scene->camera, -rotate, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        rotateCamera(scene->camera, 0.0, 0.0f, rotate);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
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

void processMouseEvents(GLFWwindow * window, float factor)
{
    double x;
    double y;

    int cx = scene->context->w / 2;
    int cy = scene->context->h / 2;

    float sensitivity = 0.05f;

    float dx, dy;

    glfwGetCursorPos(window, &x, &y);

    dx = sensitivity * ((float) cx - (float) x);
    dy = sensitivity * ((float) cy - (float) y);

    if (dx == 0.0f && dy == 0.0f)
    {
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
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

    glfwSetCursorPos(window, cx, cy);
}

void setupGLFWCallbacks(ContextSize * context)
{
    glfwSetWindowSizeCallback(context->window, windowResizeCallback);

    glfwSetInputMode(context->window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(context->window, keyboardCallback);

    glfwSetInputMode(context->window, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
    glfwSetMouseButtonCallback(context->window, mouseButtonCallback);
}

void draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawWorld(scene->world);
    drawWater(scene->water);
}

void viewFps(int frameCnt, float diffSum, const ContextSize * context)
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

    glfwSetWindowTitle(context->window, title);
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

    setupGLFWCallbacks(scene->context);
    glfwSetCursorPos(scene->context->window,
        scene->context->w / 2, scene->context->h / 2);

    while (running)
    {
        if (glfwWindowShouldClose(scene->context->window))
        {
            running = GL_FALSE;
            break;
        }

        processKeyboardEvents(scene->context->window, dSecond * BASE_FPS);
        processMouseEvents(scene->context->window, dSecond * BASE_FPS);

        ++frameCnt;
        dSecond = timeval_diff_replace(&curTime);
        dSecondSum += dSecond;

        if (dSecondSum > 0.5f)
        {
            viewFps(frameCnt, dSecondSum, scene->context);
            frameCnt = 0;
            dSecondSum = 0.0f;
        }

        if (!flags.pause)
        {
            modifyWaterMesh(scene->water, dSecond);
        }

        draw();
        glfwSwapBuffers(scene->context->window);
        glfwPollEvents();
    }

    freeScene(scene);

    return EXIT_SUCCESS;
}
