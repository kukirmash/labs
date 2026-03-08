#include "InputControl.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//--------------------------------------------------------------------------
Camera *InputControl::camera = 0;
bool InputControl::rightMousePressed = false;
int InputControl::lastMouseX = 0;
int InputControl::lastMouseY = 0;
bool InputControl::specialKeys[256] = {};

//--------------------------------------------------------------------------
void InputControl::SpecialKeyDown(int key, int x, int y)
{
    specialKeys[key] = true;
}

//--------------------------------------------------------------------------
void InputControl::SpecialKeyUp(int key, int x, int y)
{
    specialKeys[key] = false;
}

//--------------------------------------------------------------------------
// Обработка кликов мыши
void InputControl::MouseClick(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            rightMousePressed = true;
            lastMouseX = x;
            lastMouseY = y;
        }
        else
        {
            rightMousePressed = false;
        }
    }
}

//--------------------------------------------------------------------------
// Обработка движения мыши (вращение камеры)
void InputControl::MouseMotion(int x, int y)
{
    if (rightMousePressed)
    {
        float dx = (float)(x - lastMouseX);
        float dy = (float)(y - lastMouseY);

        // Поворачиваем камеру. Коэффициент 0.2f - это чувствительность мыши
        camera->Rotate(-dx * 0.05f, -dy * 0.05f);

        lastMouseX = x;
        lastMouseY = y;
    }
}

//--------------------------------------------------------------------------
// Обработка колесика мыши (зум)
void InputControl::MouseWheel(int wheel, int direction, int x, int y)
{
    // direction: +1 (вперед), -1 (назад)
    camera->Zoom(-direction * 0.5f);
}

//--------------------------------------------------------------------------