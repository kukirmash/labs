#ifndef FPSCOUNTER_H
#define FPSCOUNTER_H

#include <GL/freeglut.h>

#include <string>

//*--------------------------------------------------------------------------
class FPSCounter
{
    int frameCount;

    float fps;

    int lastTime;

    std::string windowTitle;

public:
    FPSCounter()
    {
    }
    ~FPSCounter()
    {
    }

    FPSCounter(const std::string &title = "OpenGL Window");

    void Update(); // вызывать каждый кадр

    float GetFps() // получить текущий FPS
    {
        return fps;
    }

    void SetWindowTitle(const std::string &title)
    {
        windowTitle = title;
    }

    void Reset()
    {
        lastTime = glutGet(GLUT_ELAPSED_TIME);
        frameCount = 0;
        fps = 0.0f;
    }
};

//*--------------------------------------------------------------------------
#endif