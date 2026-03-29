#ifndef GRAPHIC_OBJECT_H
#define GRAPHIC_OBJECT_H

//--------------------------------------------------------------------------
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//--------------------------------------------------------------------------
enum class GraphicObjectType
{
    none,
    road,
    building,
    vehicle,
    big_nature,
    small_nature,
    big_prop,
    medium_prop,
    small_prop
};

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

    int textureId;

    int materialId;

    GraphicObjectType type;

    glm::vec3 dimensions;

    // Вспомогательная функция для пересчета матрицы
    void UpdateModelMatrix();

public:
    GraphicObject()
    {
        color = glm::vec4(1.0f); // белый

        position = glm::vec3(0.0f, 0.0f, 0.0f);

        angle = 0.0f;

        textureId = -1;

        materialId = -1;

        dimensions = glm::vec3(1.0f);

        type = GraphicObjectType::none;

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

    void SetTextureId(int id);

    void SetMaterialId(int id);

    void SetType(GraphicObjectType t);

    void SetDimensions(glm::vec3 d);

    glm::vec3 GetPosition()
    {
        return position;
    }

    // получить различные параметры
    glm::vec4 &GetColor()
    {
        return color;
    }

    glm::mat4 &GetModelMatrix()
    {
        return modelMatrix;
    }

    int GetMeshId() const
    {
        return meshId;
    }

    int GetTextureId() const
    {
        return textureId;
    }

    int GetMaterialId() const
    {
        return materialId;
    }

    GraphicObjectType GetType()
    {
        return type;
    }

    glm::vec3 GetDimensions()
    {
        return dimensions;
    }
};

//--------------------------------------------------------------------------
#endif // GRAPHIC_OBJECT_H