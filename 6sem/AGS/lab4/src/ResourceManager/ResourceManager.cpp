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