#include "GraphicObject.h"

//--------------------------------------------------------------------------
// Вспомогательная функция для пересчета матрицы
void GraphicObject::UpdateModelMatrix()
{
    glm::mat4 m(1.0f); // Единичная матрица

    m = glm::translate(m, position);                                      // Смещение в точку
    m = glm::rotate(m, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)); // Поворот вокруг OY

    modelMatrix = m;
}

//--------------------------------------------------------------------------
// установить цвет объекта
void GraphicObject::SetColor(const glm::vec4 &c)
{
    color = c;
}

//--------------------------------------------------------------------------
// установить позицию объекта [
void GraphicObject::SetPosition(const glm::vec3 &p)
{
    position = p;
    UpdateModelMatrix();
}

//--------------------------------------------------------------------------
// установить угол поворота в градусах относительно оси ОУ по часовой стрелке
void GraphicObject::SetAngle(float a)
{
    angle = a;
    UpdateModelMatrix();
}

//--------------------------------------------------------------------------
// установить идентификатор используемого меша
void GraphicObject::SetMeshId(int id)
{
    meshId = id;
    UpdateModelMatrix();
}

//--------------------------------------------------------------------------
void GraphicObject::SetTextureId(int id)
{
    textureId = id;
}

//--------------------------------------------------------------------------