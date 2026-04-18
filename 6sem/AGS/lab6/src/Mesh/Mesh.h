#ifndef MESH_H
#define MESH_H

//--------------------------------------------------------------------------
#include <string>
#include <vector>
#include <GL/glew.h>

//--------------------------------------------------------------------------
// Структура, описывающая одну вершину
struct Vertex
{
    GLfloat coord[3];
    GLfloat normal[3];
    GLfloat texCoord[2];
};

//--------------------------------------------------------------------------
// КЛАСС ДЛЯ РАБОТЫ С МЕШЕМ
class Mesh
{
    // индекс VAO-объекта
    GLuint vao;
    // индекс VBO-буфера вершин
    GLuint vertexBuffer;
    // индекс VBO-буфера индексов
    GLuint indexBuffer;
    // количество вершин в меше
    int vertexCount;

    // Вспомогательная функция для парсинга файла
    bool ParseOBJ(const std::string &filename, std::vector<Vertex> &out_vertices, std::vector<GLuint> &out_indices);

public:
    // конструктор
    Mesh()
    {
    }
    ~Mesh()
    {
    }

    // загрузка меша из внешнего obj-файла
    bool Load(std::string filename);

    // вывод меша
    void DrawOne();
};

//--------------------------------------------------------------------------
#endif // MESH_H