#include "ResourceManager.h"
#include <iostream>

//*--------------------------------------------------------------------------
int ResourceManager::LoadMesh(std::string filename)
{
    // Проверяем, был ли меш загружен ранее
    auto it = meshesId.find(filename);
    if (it != meshesId.end())
    {
        // Отладочная информация
        std::cout << "[ResourceManager] Меш уже загружен: " << filename
                  << ". Возвращаем индекс: " << it->second << std::endl;
        return it->second;
    }

    std::cout << "[ResourceManager] Загрузка нового меша: " << filename << std::endl;

    // Создаем новый меш и пытаемся его загрузить
    Mesh newMesh;
    if (newMesh.Load(filename))
    {
        meshes.push_back(newMesh);
        int index = meshes.size() - 1;
        meshesId[filename] = index; // Запоминаем индекс загруженного файла
        return index;
    }

    std::cout << "[ResourceManager] ОШИБКА загрузки меша: " << filename << std::endl;
    return -1;
}

//*--------------------------------------------------------------------------
Mesh *ResourceManager::GetMesh(int index)
{
    // Проверка на выход за пределы вектора
    if (index >= 0 && index < meshes.size())
    {
        return &meshes[index];
    }
    return nullptr;
}

//*--------------------------------------------------------------------------
int ResourceManager::LoadTexture(std::string filename)
{
    auto it = texturesId.find(filename);
    if (it != texturesId.end())
    {
        std::cout << "[ResourceManager] Текстура уже загружена: " << filename
                  << ". Возвращаем индекс: " << it->second << std::endl;
        return it->second;
    }

    std::cout << "[ResourceManager] Загрузка новой текстуры: " << filename << std::endl;

    Texture newTexture;
    if (newTexture.Load(filename))
    {
        textures.push_back(newTexture);
        int index = textures.size() - 1;
        texturesId[filename] = index;
        return index;
    }

    std::cout << "[ResourceManager] ОШИБКА загрузки текстуры: " << filename << std::endl;
    return -1;
}

//*--------------------------------------------------------------------------
Texture *ResourceManager::GetTexture(int index)
{
    if (index >= 0 && index < textures.size())
    {
        return &textures[index];
    }
    return nullptr;
}

//*--------------------------------------------------------------------------