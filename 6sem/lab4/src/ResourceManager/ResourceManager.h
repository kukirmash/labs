#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "../Mesh/Mesh.h"
#include <vector>
#include <map>

//--------------------------------------------------------------------------
// КЛАСС ДЛЯ РАБОТЫ С МЕНЕДЖЕРОМ РЕСУРСОВ (РЕАЛИЗОВАН НА ОСНОВЕ ШАБЛОНА SINGLTON)
class ResourceManager
{
    // вектор для хранения всех мешей
    std::vector<Mesh> meshes;
    // map для хранения соответствия между именем запрашиваемого файла
    // и индексом в контейнере meshes
    std::map<std::string, int> meshesId;

    // конструктор по умолчанию (объявлен приватным)
    // в результате нельзя создать ни одного объекта данного класса вне самого класса
    ResourceManager() {};
    // конструктора копирования нет (удален)
    ResourceManager(const ResourceManager &v) = delete;
    // оператора присваивания нет (удален)
    ResourceManager &operator=(const ResourceManager &v) = delete;

public:
    ~ResourceManager()
    {
    }

    // Статик-метод для получения экземпляра менеджера ресурса.
    // Всегда будет возвращена ссылка на объект, хранящийся в единственном экземпляре.
    static ResourceManager &instance()
    {
        static ResourceManager ResourceManager;
        return ResourceManager;
    }

    // Загрузка одного меша из указанного obj-файла
    // Возвращаемое значение - индекс меша в менеджере ресурсов
    int LoadMesh(std::string filename);

    // Получение меша по его индексу
    // Если такого меша нет (недействительный индекс) возвращается nullptr
    Mesh *GetMesh(int index);
};

//--------------------------------------------------------------------------
#endif // RESOURCE_MANAGER_H