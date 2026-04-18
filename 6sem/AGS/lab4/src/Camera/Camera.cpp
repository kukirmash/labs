#include "Camera.h"

//--------------------------------------------------------------------------
// пересчитать матрицу вида
void Camera::RecalculateViewMatrix()
{
    float radTheta = glm::radians(theta);
    float radPhi = glm::radians(phi);

    // Перевод из сферических координат в декартовы для получения позиции глаза (eye)
    glm::vec3 eye;

    eye.x = center.x + radius * cos(radPhi) * sin(radTheta);
    eye.y = center.y + radius * sin(radPhi);
    eye.z = center.z + radius * cos(radPhi) * cos(radTheta);

    // примерный вектор "вверх" (0, 1, 0)
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    // матрица камеры
    viewMatrix = glm::lookAt(eye, center, up);
}

//--------------------------------------------------------------------------
// передвинуть камеру и точку наблюдения в горизонтальной плоскости (OXZ)
void Camera::MoveOXZ(float dx, float dz)
{
    float radTheta = glm::radians(theta);

    // Вектор "вперед" (с учетом направления камеры в проекции на XZ)
    glm::vec3 forwardDir(-sin(radTheta), 0.0f, -cos(radTheta));
    // Вектор "вправо"
    glm::vec3 rightDir(cos(radTheta), 0.0f, -sin(radTheta));

    // Сдвигаем точку, на которую смотрим
    center += rightDir * dx + forwardDir * dz;

    RecalculateViewMatrix();
}

//--------------------------------------------------------------------------
// повернуть в горизонтальной и вертикальной плоскости
void Camera::Rotate(float horizAngle, float vertAngle)
{
    theta += horizAngle;
    phi += vertAngle;

    // Ограничиваем вертикальный угол от 5 до 85 градусов
    if (phi < 5.0f)
        phi = 5.0f;
    if (phi > 85.0f)
        phi = 85.0f;

    RecalculateViewMatrix();
}

//--------------------------------------------------------------------------
// приблизить/удалить камеру к/от точки наблюдения
void Camera::Zoom(float dR)
{
    radius += dR;
    if (radius < 1.0f)
        radius = 1.0f; // Не даем камере провалиться сквозь центр

    RecalculateViewMatrix();
}

//--------------------------------------------------------------------------