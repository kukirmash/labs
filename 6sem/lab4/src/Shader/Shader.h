#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <map>
#include <GL/glew.h>
#include <glm/glm.hpp>

//*--------------------------------------------------------------------------
// КЛАСС ДЛЯ РАБОТЫ С ШЕЙДЕРОМ
class Shader
{
private:
    // id "текущей" выбранной шейдерной программы
    // используется чтобы повторно не активировать шейдерную программу,
    // если она уже активирована
    static GLuint currentProgram;

    // шейдерная программа (шейдер)
    GLuint program;

    // uniform-переменной - её индексом (location)
    std::map<std::string, GLuint> uniforms;

public:
    Shader()
    {
    }
    ~Shader()
    {
    }

    // загрузка шейдера из внешних файлов
    bool Load(std::string veftexShaderFilename, std::string fragmentShaderFilename);

    // выбор шейдера в качестве текущего
    void Activate()
    {
        if (program != currentProgram)
        {
            glUseProgram(program);
            currentProgram = program;
        }
    }

    // отключение шейдера
    static void Deactivate()
    {
        glUseProgram(0);
    }

    // установка uniform-переменных
    void SetUniform(std::string name, int value);
    void SetUniform(std::string name, float value);
    void SetUniform(std::string name, glm::vec2 &value);
    void SetUniform(std::string name, glm::vec4 &value);
    void SetUniform(std::string name, glm::mat4 &value);

private:
    // создание шейдерного объекта указанного типа
    // и загрузка исходного текста шейдера из указанного файла
    GLuint CreateShaderObject(GLenum shaderType, std::string filename);

    // получение индекса (location) uniform-переменной
    GLuint GetUniformLocation(std::string name);
};

//*--------------------------------------------------------------------------

#endif