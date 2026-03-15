// FPSCounter.cpp
#include "FPSCounter.h"

#include <cstdio>

//*--------------------------------------------------------------------------
FPSCounter::FPSCounter(const std::string &title)
{
    frameCount = 0;
    fps = 0.0f;
    windowTitle = title;
    lastTime = glutGet(GLUT_ELAPSED_TIME);
}

//*--------------------------------------------------------------------------
void FPSCounter::Update()
{
    frameCount++;

    int currentTime = glutGet(GLUT_ELAPSED_TIME); // мс
    int timeInterval = currentTime - lastTime;

    // Обновляем заголовок раз в секунду
    if (timeInterval >= 1000)
    {
        fps = frameCount * 1000.f / timeInterval;

        char title[256];
        sprintf(title, "%s | FPS: %d", windowTitle.c_str(), (int)fps);
        glutSetWindowTitle(title);

        lastTime = currentTime;
        frameCount = 0;
    }
}

//*--------------------------------------------------------------------------
