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

/* ==== Globals ==== */

typedef
struct BuriedGlobals
{
    GLboolean pause;
    GLboolean vsync;
    /* running -- is application running (GL_TRUE) or must be terminated (GL_FALSE) */
    GLboolean running;
    Scene * scene;
}
BuriedGlobals;

BuriedGlobals * newBuriedGlobals()
{
    BuriedGlobals * globals = (BuriedGlobals *) malloc(sizeof(BuriedGlobals));
    globals->pause = GL_FALSE;
    globals->vsync = GL_TRUE;
    globals->running = GL_TRUE;
    globals->scene = newScene(globals->vsync);
    glfwSetWindowUserPointer(globals->scene->context->window, globals);
    return globals;
}

/* ==== Callbacks ==== */

void windowResizeCallback(GLFWwindow * window, int w, int h)
{
    BuriedGlobals * globals = (BuriedGlobals *) glfwGetWindowUserPointer(window);

    globals->scene->context->w = w;
    globals->scene->context->h = h;

    globals->scene->camera->aspect =
        ((float) globals->scene->context->w) /
        ((float) globals->scene->context->h);

    if (globals->scene->world != NULL)
    {
        setupCamera(globals->scene->world->sp, globals->scene->camera);
    }

    if (globals->scene->water != NULL)
    {
        setupCameraAlter(globals->scene->water->modifySP,
            globals->scene->camera);
        setupCamera(globals->scene->water->drawSP,
            globals->scene->camera);
    }

    glViewport(0, 0, w, h);
}

void keyboardCallback(GLFWwindow * window, int key, int scancode,
    int action, int mods)
{
    BuriedGlobals * globals = (BuriedGlobals *) glfwGetWindowUserPointer(window);

    UNUSED(scancode);
    UNUSED(mods);

    if (key == GLFW_KEY_PAUSE && action == GLFW_PRESS)
    {
        globals->pause = !globals->pause;
    }

    if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
    {
        globals->vsync = !globals->vsync;

        if (globals->vsync)
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
    BuriedGlobals * globals = (BuriedGlobals *) glfwGetWindowUserPointer(window);

    UNUSED(mods);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS &&
        !globals->pause)
    {
        setWaterWave(globals->scene->water);
    }
}

void processKeyboardEvents(GLFWwindow * window, float factor)
{
    BuriedGlobals * globals = (BuriedGlobals *) glfwGetWindowUserPointer(window);

    int cameraModified = 0;
    float slide = SLIDE_STEP * factor;
    float rotate = ROTATE_STEP * factor;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        globals->running = GL_FALSE;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        slideCamera(globals->scene->camera, 0.0f, 0.0f, -slide);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        slideCamera(globals->scene->camera, 0.0f, 0.0f, slide);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        slideCamera(globals->scene->camera, -slide, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        slideCamera(globals->scene->camera, slide, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
    {
        slideCamera(globals->scene->camera, 0.0f, slide, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
    {
        slideCamera(globals->scene->camera, 0.0f, -slide, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        rotateCamera(globals->scene->camera, 0.0f, -rotate, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        rotateCamera(globals->scene->camera, 0.0f, rotate, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        rotateCamera(globals->scene->camera, rotate, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        rotateCamera(globals->scene->camera, -rotate, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        rotateCamera(globals->scene->camera, 0.0, 0.0f, rotate);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        rotateCamera(globals->scene->camera, 0.0f, 0.0f, -rotate);
        cameraModified = 1;
    }

    if (cameraModified && globals->scene->world != NULL)
    {
        setupCamera(globals->scene->world->sp, globals->scene->camera);
    }

    if (cameraModified && globals->scene->water != NULL)
    {
        setupCameraAlter(globals->scene->water->modifySP,
            globals->scene->camera);
        setupCamera(globals->scene->water->drawSP,
            globals->scene->camera);
    }
}

void processMouseEvents(GLFWwindow * window, float factor)
{
    BuriedGlobals * globals = (BuriedGlobals *) glfwGetWindowUserPointer(window);

    double x;
    double y;

    int cx = globals->scene->context->w / 2;
    int cy = globals->scene->context->h / 2;

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

        slideCamera(globals->scene->camera, -dx, dy, 0.0f);
    }
    else
    {
        float rotate = ROTATE_STEP * factor;

        dx *= rotate;
        dy *= rotate;

        rotateCamera(globals->scene->camera, dx, 0.0f, 0.0f);
        rotateCamera(globals->scene->camera, 0.0f, -dy, 0.0f);
    }

    if (globals->scene->world != NULL)
    {
        setupCamera(globals->scene->world->sp, globals->scene->camera);
    }

    if (globals->scene->water != NULL)
    {
        setupCameraAlter(globals->scene->water->modifySP,
            globals->scene->camera);
        setupCamera(globals->scene->water->drawSP,
            globals->scene->camera);
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

void draw(const Scene * scene)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawWorld(scene->world);
    drawWater(scene->water);
}

void viewFps(int frameCnt, float diffSum, const BuriedGlobals * globals)
{
    static char title[64];
    float fps = frameCnt / diffSum;

    if (globals->vsync && globals->pause)
    {
        sprintf(title, "Wave Simulation; FPS: %0.0f [vsync, paused]", fps);
    }
    else if (globals->vsync)
    {
        sprintf(title, "Wave Simulation; FPS: %0.0f [vsync]", fps);
    }
    else if (globals->pause)
    {
        sprintf(title, "Wave Simulation; FPS: %0.0f [paused]", fps);
    }
    else
    {
        sprintf(title, "Wave Simulation; FPS: %0.0f", fps);
    }

    glfwSetWindowTitle(globals->scene->context->window, title);
}

/* ==== Main ==== */

int main()
{
    BuriedGlobals * globals = newBuriedGlobals();

    int frameCnt = 0;
    struct timeval curTime;
    float dSecond = 0;
    float dSecondSum = 0; /* droped if more than 0.5 sec */

    timeval_diff_replace(&curTime);

    setupGLFWCallbacks(globals->scene->context);
    glfwSetCursorPos(globals->scene->context->window,
        globals->scene->context->w / 2, globals->scene->context->h / 2);

    while (globals->running)
    {
        if (glfwWindowShouldClose(globals->scene->context->window))
        {
            globals->running = GL_FALSE;
            break;
        }

        processKeyboardEvents(globals->scene->context->window,
            dSecond * BASE_FPS);
        processMouseEvents(globals->scene->context->window,
            dSecond * BASE_FPS);

        ++frameCnt;
        dSecond = timeval_diff_replace(&curTime);
        dSecondSum += dSecond;

        if (dSecondSum > 0.5f)
        {
            viewFps(frameCnt, dSecondSum, globals);
            frameCnt = 0;
            dSecondSum = 0.0f;
        }

        if (!globals->pause)
        {
            modifyWaterMesh(globals->scene->water, dSecond);
        }

        draw(globals->scene);
        glfwSwapBuffers(globals->scene->context->window);
        glfwPollEvents();
    }

    freeScene(globals->scene);
    free(globals);

    return EXIT_SUCCESS;
}
