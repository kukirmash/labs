#ifndef TEXTURE_H
#define TEXTURE_H

//--------------------------------------------------------------------------
#include <string>
#include <GL/glew.h>

//--------------------------------------------------------------------------
// КЛАСС ДЛЯ РАБОТЫ С ТЕКСТУРОЙ
class Texture
{
    // индекс текстурного объекта
    GLuint texIndex;

public:
    Texture() : texIndex(0) {}
    ~Texture() {}

    // загрузка текстуры из внешнего файла
    bool Load(const std::string &filename);

    // применение текстуры (привязка к текстурному блоку)
    void Bind(GLenum texUnit = GL_TEXTURE0);
};

//--------------------------------------------------------------------------
#endif // TEXTURE_H