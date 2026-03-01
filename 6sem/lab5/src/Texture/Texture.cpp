#include "Texture.h"

//--------------------------------------------------------------------------
#include <iostream>
#include <IL/il.h>

//--------------------------------------------------------------------------
bool Texture::Load(const std::string &filename)
{
    ILuint imgName;
    ilGenImages(1, &imgName);
    ilBindImage(imgName);

    // Загружаем изображение
    if (!ilLoadImage(filename.c_str()))
    {
        std::cerr << "Не удалось загрузить текстуру: " << filename << std::endl;
        ilDeleteImages(1, &imgName);
        return false;
    }

    // Конвертируем в удобный для OpenGL формат (RGBA, байты)
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    // Создаем текстурный объект OpenGL
    glGenTextures(1, &texIndex);
    // Делаем нулевой текстурный блок активным
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texIndex);

    // Устанавливаем параметры (включая линейную фильтрацию)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Загружаем данные в видеокарту
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
                 ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());

    // Генерируем мипмапы
    glGenerateMipmap(GL_TEXTURE_2D);

    // Отвязываем текстуру и очищаем память DevIL
    glBindTexture(GL_TEXTURE_2D, 0);
    ilDeleteImages(1, &imgName);

    return true;
}

//--------------------------------------------------------------------------
void Texture::Bind(GLenum texUnit)
{
    glActiveTexture(texUnit);
    glBindTexture(GL_TEXTURE_2D, texIndex);
}

//--------------------------------------------------------------------------