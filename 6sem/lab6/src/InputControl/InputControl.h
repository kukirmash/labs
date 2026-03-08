#ifndef INPUT_CONTROL_H
#define INPUT_CONTROL_H

#include "../Camera/Camera.h"

//--------------------------------------------------------------------------
class InputControl
{
    // Переменные для отслеживания мыши
    static bool rightMousePressed;
    static int lastMouseX;
    static int lastMouseY;

public:
    static Camera *camera;
    // Массивы для отслеживания зажатых клавиш
    static bool specialKeys[256];

public:
    InputControl()
    {
    }
    ~InputControl()
    {
    }

    static void SpecialKeyDown(int key, int x, int y);

    static void SpecialKeyUp(int key, int x, int y);

    static void MouseClick(int button, int state, int x, int y);

    static void MouseMotion(int x, int y);

    static void MouseWheel(int wheel, int direction, int x, int y);
};

//--------------------------------------------------------------------------

#endif // INPUT_CONTROL_H