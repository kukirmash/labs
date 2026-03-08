#ifndef MATERIAL_H
#define MATERIAL_H

//--------------------------------------------------------------------------
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>

//--------------------------------------------------------------------------
// КЛАСС ДЛЯ РАБОТЫ С МАТЕРИАЛОМ
class Material
{
public:
    // фоновая составляющая
    glm::vec4 ambient;
    // диффузная составляющая
    glm::vec4 diffuse;
    // зеркальная составляющая
    glm::vec4 specular;
    // степень отполированности
    GLfloat shininess;

public:
    // конструктор по умолчанию
    Material() : ambient(1.0f), diffuse(1.0f), specular(1.0f), shininess(32.0f) {}
    ~Material()
    {
    }

    // загрузка параметров материала из json-файла
    bool LoadFromJson(const std::string &filename);

    // получение различных параметров материала
    glm::vec4 &GetAmbient();
    glm::vec4 &GetDiffuse();
    glm::vec4 &GetSpecular();
    float GetShininess();
};

//--------------------------------------------------------------------------
#endif // MATERIAL_H