#include "Mesh.h"

//--------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <glm/glm.hpp>

//--------------------------------------------------------------------------
// Парсер .obj файлов
bool Mesh::ParseOBJ(const std::string &filename, std::vector<Vertex> &outVertices, std::vector<GLuint> &outIndices)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Не удалось открыть файл: " << filename << std::endl;
        return false;
    }

    // Временные массивы для сырых данных из файла
    std::vector<glm::vec3> tempPositions;
    std::vector<glm::vec2> tempTexcoords;
    std::vector<glm::vec3> tempNormals;

    // Словарь для отслеживания уникальных вершин (по строке формата "v/vt/vn")
    std::map<std::string, GLuint> uniqueVertices;

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v")
        {
            glm::vec3 p;
            iss >> p.x >> p.y >> p.z;
            tempPositions.push_back(p);
        }
        else if (type == "vt")
        {
            glm::vec2 t;
            iss >> t.x >> t.y;
            tempTexcoords.push_back(t);
        }
        else if (type == "vn")
        {
            glm::vec3 n;
            iss >> n.x >> n.y >> n.z;
            tempNormals.push_back(n);
        }
        else if (type == "f")
        {
            // Читаем 3 вершины полигона (предполагаем, что меш триангулирован)
            std::string vertexData[3];
            iss >> vertexData[0] >> vertexData[1] >> vertexData[2];

            for (int i = 0; i < 3; ++i)
            {
                // Если мы еще не встречали такую комбинацию v/vt/vn
                if (uniqueVertices.count(vertexData[i]) == 0)
                {
                    GLuint vIdx = 0, vtIdx = 0, vnIdx = 0;

                    // Парсим строку вида "v/vt/vn" или "v//vn"
                    // В .obj индексы начинаются с 1, поэтому отнимаем 1
                    if (vertexData[i].find("//") != std::string::npos)
                    {
                        sscanf(vertexData[i].c_str(), "%d//%d", &vIdx, &vnIdx);
                        vtIdx = 1; // Заглушка, если нет текстурных координат
                    }
                    else
                    {
                        sscanf(vertexData[i].c_str(), "%d/%d/%d", &vIdx, &vtIdx, &vnIdx);
                    }

                    Vertex vertex;

                    // Геометрические координаты
                    vertex.coord[0] = tempPositions[vIdx - 1].x;
                    vertex.coord[1] = tempPositions[vIdx - 1].y;
                    vertex.coord[2] = tempPositions[vIdx - 1].z;

                    // Текстурные координаты (если они есть в файле)
                    if (!tempTexcoords.empty() && vtIdx > 0)
                    {
                        vertex.texCoord[0] = tempTexcoords[vtIdx - 1].x;
                        vertex.texCoord[1] = tempTexcoords[vtIdx - 1].y;
                    }
                    else
                    {
                        vertex.texCoord[0] = 0.0f;
                        vertex.texCoord[1] = 0.0f;
                    }

                    // Нормали
                    if (!tempNormals.empty() && vnIdx > 0)
                    {
                        vertex.normal[0] = tempNormals[vnIdx - 1].x;
                        vertex.normal[1] = tempNormals[vnIdx - 1].y;
                        vertex.normal[2] = tempNormals[vnIdx - 1].z;
                    }
                    else
                    {
                        vertex.normal[0] = 0.0f;
                        vertex.normal[1] = 1.0f;
                        vertex.normal[2] = 0.0f;
                    }

                    // Сохраняем новую вершину
                    outVertices.push_back(vertex);
                    GLuint newIndex = static_cast<GLuint>(outVertices.size() - 1);
                    uniqueVertices[vertexData[i]] = newIndex;
                    outIndices.push_back(newIndex);
                }
                else
                {
                    // Если вершина уже есть, просто добавляем ее индекс
                    outIndices.push_back(uniqueVertices[vertexData[i]]);
                }
            }
        }
    }
    return true;
}

//--------------------------------------------------------------------------
bool Mesh::Load(std::string filename)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    if (!ParseOBJ(filename, vertices, indices))
        return false;

    vertexCount = indices.size();

    // 1. Создание и привязка VAO-объекта
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // 2. Создание и инициализация буфера вершин
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // 3. Создание буфера индексов
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // 4. Указание места и формата хранения атрибутов вершин
    // Координаты (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

    // Нормали (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(3 * sizeof(GLfloat)));

    // Текстурные координаты (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(6 * sizeof(GLfloat)));

    // 5. "Отвязка" VAO-объекта
    glBindVertexArray(0);

    return true;
}

//--------------------------------------------------------------------------
void Mesh::DrawOne()
{
    // 1. Привязка VAO-объекта
    glBindVertexArray(vao);

    // 2. Вызов команды отрисовки полигонов (draw call)
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);

    // 3. "Отвязка" VAO-объекта
    glBindVertexArray(0);
}

//--------------------------------------------------------------------------