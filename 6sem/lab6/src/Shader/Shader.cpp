// Shader.cpp
#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;
using namespace glm;

GLuint Shader::currentProgram = 0;

//*--------------------------------------------------------------------------
// загрузка шейдера из внешних файлов
bool Shader::Load(std::string vertexShaderFilename, std::string fragmentShaderFilename)
{
    // создаем объекты вершинного и фрагментного шейдера
    GLuint vertexShader = this->CreateShaderObject(GL_VERTEX_SHADER, vertexShaderFilename);
    GLuint fragmentShader = this->CreateShaderObject(GL_FRAGMENT_SHADER, fragmentShaderFilename);

    // проверка на ошибки загрузки шейдеров
    if (vertexShader == 0 || fragmentShader == 0)
        return false;

    // создаем шейдерную программу
    program = glCreateProgram();

    // прикрепляем шейдеры к программе
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // линковка программы
    glLinkProgram(program);

    // проверка на ошибки линковки
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength + 1);
        glGetProgramInfoLog(program, infoLogLength, NULL, &infoLog[0]);

        std::cerr << "Shader program linking error: " << &infoLog[0] << std::endl;

        glDeleteProgram(program);
        return false;
    }

    // после линковки шейдеры можно удалить
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    printf("Shaders loaded successfully: %s %s\n", vertexShaderFilename.c_str(), fragmentShaderFilename.c_str());

    return true;
}

//*--------------------------------------------------------------------------
// установка uniform-переменных
void Shader::SetUniform(std::string name, int value)
{
    GLuint loc = GetUniformLocation(name);
    glUniform1i(loc, value);
}

void Shader::SetUniform(std::string name, float value)
{
    GLuint loc = GetUniformLocation(name);
    glUniform1f(loc, value);
}

void Shader::SetUniform(std::string name, const glm::vec2 &value)
{
    GLuint loc = GetUniformLocation(name);
    glUniform2fv(loc, 1, glm::value_ptr(value));
}

void Shader::SetUniform(std::string name, const glm::vec3 &value)
{
    GLuint loc = GetUniformLocation(name);
    glUniform3fv(loc, 1, glm::value_ptr(value));
}

void Shader::SetUniform(std::string name, const glm::vec4 &value)
{
    GLuint loc = GetUniformLocation(name);
    glUniform4fv(loc, 1, glm::value_ptr(value));
}

void Shader::SetUniform(std::string name, const glm::mat4 &value)
{
    GLuint loc = GetUniformLocation(name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

//*--------------------------------------------------------------------------
// создание шейдерного объекта указанного типа
// и загрузка исходного текста шейдера из указанного файла
GLuint Shader::CreateShaderObject(GLenum shaderType, std::string filename)
{
    // открываем файл с исходным кодом шейдера
    ifstream file(filename.c_str());
    if (!file.is_open())
    {
        cerr << "Failed to open shader file: " << filename << endl;
        return 0;
    }

    // читаем весь файл в строку
    stringstream ss;
    ss << file.rdbuf();
    string shaderSource = ss.str();
    file.close();

    // создаем шейдерный объект
    GLuint shader = glCreateShader(shaderType);

    // передаем исходный код в шейдер
    const char *source = shaderSource.c_str();
    glShaderSource(shader, 1, &source, NULL);

    // компилируем шейдер
    glCompileShader(shader);

    // проверяем результат компиляции
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus != GL_TRUE)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        vector<GLchar> infoLog(infoLogLength + 1);
        glGetShaderInfoLog(shader, infoLogLength, NULL, &infoLog[0]);

        cerr << "Shader compilation error in " << filename << ": " << &infoLog[0] << endl;

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

//*--------------------------------------------------------------------------
// получение индекса (location) uniform-переменной
GLuint Shader::GetUniformLocation(std::string name)
{
    // поиск в std::map
    auto it = uniforms.find(name);

    if (it != uniforms.end())
        return it->second; // нашли - возвращаем

    // не нашли - получаем от OpenGL
    GLuint location = glGetUniformLocation(program, name.c_str());

    // сохраняем на будущее
    uniforms[name] = location;

    return location;
}

//*--------------------------------------------------------------------------