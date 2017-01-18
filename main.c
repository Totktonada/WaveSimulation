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
struct {
    GLboolean vsync;
    GLboolean running;
    GLboolean pause;
    Scene * scene;
} Controller;

/* ==== Callbacks ==== */

void windowResizeCallback(GLFWwindow * window, int w, int h)
{
    Controller * controller;

    controller = (Controller*)glfwGetWindowUserPointer(window);

    controller->scene->context->w = w;
    controller->scene->context->h = h;

    controller->scene->camera->aspect =
        ((float) controller->scene->context->w) /
        ((float) controller->scene->context->h);

    if (controller->scene->world != NULL)
    {
        setupCamera(controller->scene->world->sp, controller->scene->camera);
    }

    if (controller->scene->water != NULL)
    {
        setupCameraAlter(controller->scene->water->modifySP,
                         controller->scene->camera);
        setupCamera(controller->scene->water->drawSP,
                    controller->scene->camera);
    }

    glViewport(0, 0, w, h);
}

void keyboardCallback(GLFWwindow * window, int key, int scancode,
    int action, int mods)
{
    Controller * controller;

    UNUSED(scancode);
    UNUSED(mods);

    controller = (Controller*)glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_PAUSE && action == GLFW_PRESS)
    {
        controller->pause = !controller->pause;
    }

    if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
    {
        controller->vsync = !controller->vsync;

        if (controller->vsync)
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
    Controller * controller;

    UNUSED(mods);

    controller = (Controller*)glfwGetWindowUserPointer(window);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS &&
        !controller->pause)
    {
        setWaterWave(controller->scene->water);
    }
}

void processKeyboardEvents(GLFWwindow * window, float factor)
{
    int cameraModified = 0;
    float slide = SLIDE_STEP * factor;
    float rotate = ROTATE_STEP * factor;
    Controller * controller;

    controller = (Controller*)glfwGetWindowUserPointer(window);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        controller->running = GL_FALSE;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        slideCamera(controller->scene->camera, 0.0f, 0.0f, -slide);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        slideCamera(controller->scene->camera, 0.0f, 0.0f, slide);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        slideCamera(controller->scene->camera, -slide, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        slideCamera(controller->scene->camera, slide, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
    {
        slideCamera(controller->scene->camera, 0.0f, slide, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
    {
        slideCamera(controller->scene->camera, 0.0f, -slide, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        rotateCamera(controller->scene->camera, 0.0f, -rotate, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        rotateCamera(controller->scene->camera, 0.0f, rotate, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        rotateCamera(controller->scene->camera, rotate, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        rotateCamera(controller->scene->camera, -rotate, 0.0f, 0.0f);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        rotateCamera(controller->scene->camera, 0.0, 0.0f, rotate);
        cameraModified = 1;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        rotateCamera(controller->scene->camera, 0.0f, 0.0f, -rotate);
        cameraModified = 1;
    }

    if (cameraModified && controller->scene->world != NULL)
    {
        setupCamera(controller->scene->world->sp, controller->scene->camera);
    }

    if (cameraModified && controller->scene->water != NULL)
    {
        setupCameraAlter(controller->scene->water->modifySP,
                         controller->scene->camera);
        setupCamera(controller->scene->water->drawSP,
                    controller->scene->camera);
    }
}

void processMouseEvents(GLFWwindow * window, float factor)
{
    double x;
    double y;

    int cx;
    int cy;

    float sensitivity = 0.05f;

    float dx, dy;

    Controller * controller;

    controller = (Controller*)glfwGetWindowUserPointer(window);

    glfwGetCursorPos(window, &x, &y);

    cx = controller->scene->context->w / 2;
    cy = controller->scene->context->h / 2;
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

        slideCamera(controller->scene->camera, -dx, dy, 0.0f);
    }
    else
    {
        float rotate = ROTATE_STEP * factor;

        dx *= rotate;
        dy *= rotate;

        rotateCamera(controller->scene->camera, dx, 0.0f, 0.0f);
        rotateCamera(controller->scene->camera, 0.0f, -dy, 0.0f);
    }

    if (controller->scene->world != NULL)
    {
        setupCamera(controller->scene->world->sp, controller->scene->camera);
    }

    if (controller->scene->water != NULL)
    {
        setupCameraAlter(controller->scene->water->modifySP,
                         controller->scene->camera);
        setupCamera(controller->scene->water->drawSP,
                    controller->scene->camera);
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

void draw(const Controller * controller)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawWorld(controller->scene->world);
    drawWater(controller->scene->water);
}

void viewFps(int frameCnt, float diffSum, const Controller * controller)
{
    static char title[64];
    float fps = frameCnt / diffSum;

    if (controller->vsync && controller->pause)
    {
        sprintf(title, "Wave Simulation; FPS: %0.0f [vsync, paused]", fps);
    }
    else if (controller->vsync)
    {
        sprintf(title, "Wave Simulation; FPS: %0.0f [vsync]", fps);
    }
    else if (controller->pause)
    {
        sprintf(title, "Wave Simulation; FPS: %0.0f [paused]", fps);
    }
    else
    {
        sprintf(title, "Wave Simulation; FPS: %0.0f", fps);
    }

    glfwSetWindowTitle(controller->scene->context->window, title);
}

/* ==== Main ==== */

int main()
{
    Controller controller;
    int frameCnt = 0;
    struct timeval curTime;
    float dSecond = 0;
    float dSecondSum = 0; /* droped if more than 0.5 sec */

    timeval_diff_replace(&curTime);

    controller.vsync = GL_TRUE;
    controller.running = GL_TRUE;
    controller.pause = GL_FALSE;
    controller.scene = newScene(controller.vsync);
    glfwSetWindowUserPointer(controller.scene->context->window, &controller);

    setupGLFWCallbacks(controller.scene->context);
    glfwSetCursorPos(controller.scene->context->window,
        controller.scene->context->w / 2, controller.scene->context->h / 2);

    while (controller.running)
    {
        if (glfwWindowShouldClose(controller.scene->context->window))
        {
            controller.running = GL_FALSE;
            break;
        }

        processKeyboardEvents(controller.scene->context->window,
                              dSecond * BASE_FPS);
        processMouseEvents(controller.scene->context->window,
                           dSecond * BASE_FPS);

        ++frameCnt;
        dSecond = timeval_diff_replace(&curTime);
        dSecondSum += dSecond;

        if (dSecondSum > 0.5f)
        {
            viewFps(frameCnt, dSecondSum, &controller);
            frameCnt = 0;
            dSecondSum = 0.0f;
        }

        if (!controller.pause)
        {
            modifyWaterMesh(controller.scene->water, dSecond);
        }

        draw(&controller);
        glfwSwapBuffers(controller.scene->context->window);
        glfwPollEvents();
    }

    freeScene(controller.scene);

    return EXIT_SUCCESS;
}
