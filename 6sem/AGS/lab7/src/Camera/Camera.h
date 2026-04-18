#ifndef CAMERA_H
#define CAMERA_H

//--------------------------------------------------------------------------
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//--------------------------------------------------------------------------
// КЛАСС ДЛЯ РАБОТЫ С КАМЕРОЙ
class Camera
{
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;

    // необходимые приватные поля класса
    glm::vec3 center;
    float radius;
    float theta; // азимут
    float phi;   // высота

    // пересчитать матрицу вида
    void RecalculateViewMatrix();

public:
    // конструктор по умолчанию
    Camera()
    {
        center = glm::vec3(0.0f, 0.0f, 0.0f); // Точка наблюдения
        radius = 5.0f;                        // Расстояние до точки
        theta = 0.0f;                         // Горизонтальный угол
        phi = 30.0f;                          // Вертикальный угол

        RecalculateViewMatrix();
    }

    // передвинуть камеру и точку наблюдения в горизонтальной плоскости (OXZ)
    void MoveOXZ(float dx, float dz);

    // повернуть в горизонтальной и вертикальной плоскости
    void Rotate(float horizAngle, float vertAngle);

    // приблизить/удалить камеру к/от точки наблюдения
    void Zoom(float dR);

    // установить матрицу проекции
    void SetProjectionMatrix(float fovy, float aspect, float zNear, float zFar)
    {
        projectionMatrix = glm::perspective(glm::radians(fovy), aspect, zNear, zFar);
    }

    // получить матрицу проекции
    glm::mat4 &GetProjectionMatrix()
    {
        return projectionMatrix;
    }

    // получить матрицу вида
    glm::mat4 &GetViewMatrix()
    {
        return viewMatrix;
    }

    glm::vec3 GetPosition()
    {
        // Извлекаем позицию из 4-го столбца обратной матрицы вида
        return glm::vec3(glm::inverse(viewMatrix)[3]);
    }
};

//--------------------------------------------------------------------------
#endif // CAMERA_H