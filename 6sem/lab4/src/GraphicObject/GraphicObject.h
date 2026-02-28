#ifndef GRAPHIC_OBJECT_H
#define GRAPHIC_OBJECT_H

//--------------------------------------------------------------------------
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//--------------------------------------------------------------------------
// КЛАСС ДЛЯ РАБОТЫ С ГРАФИЧЕСКИМ ОБЪЕКТОМ
class GraphicObject
{
    glm::vec4 color;       // цвет объекта
    glm::mat4 modelMatrix; // матрица модели (задает позицию и ориентацию)

    // Дополнительные приватные поля для удобного перерасчета
    glm::vec3 position;
    float angle;

    // идентификатор используемого меша
    int meshId;

    // Вспомогательная функция для пересчета матрицы
    void UpdateModelMatrix();

public:
    GraphicObject()
    {
        color = glm::vec4(1.0f); // белый

        position = glm::vec3(0.0f, 0.0f, 0.0f);

        angle = 0.0f;

        UpdateModelMatrix();
    }

    ~GraphicObject()
    {
    }

    // установить цвет объекта
    void SetColor(const glm::vec4 &c);

    // установить позицию объекта [
    void SetPosition(const glm::vec3 &p);

    // установить угол поворота в градусах относительно оси ОY по часовой стрелке
    void SetAngle(float a);

    // установить идентификатор используемого меша
    void SetMeshId(int id);

    // получить различные параметры
    glm::vec4 &GetColor()
    {
        return color;
    }

    glm::mat4 &GetModelMatrix()
    {
        return modelMatrix;
    }

    int GetMeshId()
    {
        return meshId;
    }
};

//--------------------------------------------------------------------------
#endif // GRAPHIC_OBJECT_H