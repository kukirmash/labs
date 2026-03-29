#include "Scene.h"

#include "../ResourceManager/ResourceManager.h"
#include "../RenderManager/RenderManager.h"
#include <fstream>
#include <iostream>
#include <rapidjson/istreamwrapper.h>

//--------------------------------------------------------------------------
void Scene::Init(std::string filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open())
    {
        std::cerr << "Не удалось открыть файл моделей: " << filename << std::endl;
        return;
    }
    rapidjson::IStreamWrapper isw(ifs);
    document.ParseStream(isw);

    if (document.HasParseError())
    {
        std::cerr << "ОШИБКА ПАРСИНГА в файле " << filename << " (возможно, лишняя запятая или забытая скобка)!" << std::endl;
    }
    else
    {
        std::cout << "Файл " << filename << " успешно прочитан. Найдено моделей: " << document.MemberCount() << std::endl;
    }
}

//--------------------------------------------------------------------------
GraphicObject Scene::CreateGraphicObject(std::string model)
{
    GraphicObject obj;
    ResourceManager &rm = ResourceManager::instance();

    if (document.HasMember(model.c_str()))
    {
        const rapidjson::Value &data = document[model.c_str()];

        // Читаем тип [cite: 32-60]
        std::string typeStr = data["type"].GetString();
        if (typeStr == "road")
            obj.SetType(GraphicObjectType::road);
        else if (typeStr == "building")
            obj.SetType(GraphicObjectType::building);
        else if (typeStr == "vehicle")
            obj.SetType(GraphicObjectType::vehicle);
        else if (typeStr == "big nature")
            obj.SetType(GraphicObjectType::big_nature);
        else if (typeStr == "small nature")
            obj.SetType(GraphicObjectType::small_nature);
        else if (typeStr == "big prop")
            obj.SetType(GraphicObjectType::big_prop);
        else if (typeStr == "medium prop")
            obj.SetType(GraphicObjectType::medium_prop);
        else if (typeStr == "small prop")
            obj.SetType(GraphicObjectType::small_prop);

        // Читаем габариты
        glm::vec3 dim(1.0f);
        if (data.HasMember("dimensions"))
        {
            dim.x = data["dimensions"][0].GetFloat();
            dim.y = data["dimensions"][1].GetFloat();
            dim.z = data["dimensions"][2].GetFloat();
        }
        obj.SetDimensions(dim);

        std::string meshPath = "../" + std::string(data["mesh"].GetString());
        std::string texPath = "../" + std::string(data["texture"].GetString());
        std::string matPath = "../" + std::string(data["material"].GetString());

        obj.SetMeshId(rm.LoadMesh(meshPath));
        obj.SetTextureId(rm.LoadTexture(texPath));
        obj.SetMaterialId(rm.LoadMaterial(matPath));
    }
    else
    {
        std::cerr << "ОШИБКА: Модель '" << model << "' не найдена в models.json!" << std::endl;
    }

    return obj;
}

//--------------------------------------------------------------------------
bool Scene::LoadFromJson(std::string filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open())
        return false;

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document sceneDoc;
    sceneDoc.ParseStream(isw);

    if (sceneDoc.HasParseError())
    {
        std::cerr << "ОШИБКА ПАРСИНГА в файле " << filename << "!" << std::endl;
    }

    graphicObjects.clear();

    if (sceneDoc.IsArray())
    {
        for (rapidjson::SizeType i = 0; i < sceneDoc.Size(); i++)
        {
            const rapidjson::Value &item = sceneDoc[i];

            std::string modelName = item["model"].GetString();
            GraphicObject obj = CreateGraphicObject(modelName);

            glm::vec3 pos(0.0f);
            pos.x = item["position"][0].GetFloat();
            pos.y = item["position"][1].GetFloat();
            pos.z = item["position"][2].GetFloat();
            obj.SetPosition(pos);

            obj.SetAngle(item["angle"].GetFloat());

            graphicObjects.push_back(obj);
        }
    }
    else
    {
        std::cerr << "ОШИБКА: Файл " << filename << " не является массивом (он должен начинаться с '[')!" << std::endl;
    }
    return true;
}

//--------------------------------------------------------------------------
void Scene::SetCamera(Camera *cam)
{
    camera = cam;
}
void Scene::SetLight(Light *l)
{
    light = l;
}

//--------------------------------------------------------------------------
bool Scene::LodTest(GraphicObject &obj)
{
    if (!camera)
        return true;
    float dist = glm::distance(camera->GetPosition(), obj.GetPosition());

    // [cite: 367]
    switch (obj.GetType())
    {
    case GraphicObjectType::road:
    case GraphicObjectType::building:
        return true;
    case GraphicObjectType::vehicle:
        return dist <= 500.0f;
    case GraphicObjectType::big_nature:
        return dist <= 350.0f;
    case GraphicObjectType::big_prop:
        return dist <= 400.0f;
    case GraphicObjectType::medium_prop:
        return dist <= 300.0f;
    case GraphicObjectType::small_nature:
    case GraphicObjectType::small_prop:
        return dist <= 200.0f;
    default:
        return true;
    }
}

//--------------------------------------------------------------------------
bool Scene::FrustumCullingTest(GraphicObject &obj)
{
    if (!camera)
        return true;

    glm::mat4 PVM = camera->GetProjectionMatrix() * camera->GetViewMatrix() * obj.GetModelMatrix();
    glm::vec3 dim = obj.GetDimensions();
    float dx = dim.x / 2.0f;
    float dy = dim.y / 2.0f;
    float dz = dim.z / 2.0f;

    glm::vec4 v[8] = {
        glm::vec4(dx, dy, dz, 1.0f), glm::vec4(dx, -dy, dz, 1.0f),
        glm::vec4(dx, -dy, -dz, 1.0f), glm::vec4(dx, dy, -dz, 1.0f),
        glm::vec4(-dx, dy, dz, 1.0f), glm::vec4(-dx, -dy, dz, 1.0f),
        glm::vec4(-dx, -dy, -dz, 1.0f), glm::vec4(-dx, dy, -dz, 1.0f)};

    int out[6] = {0, 0, 0, 0, 0, 0};

    for (int i = 0; i < 8; i++)
    {
        glm::vec4 c = PVM * v[i];
        if (c.x > c.w)
            out[0]++;
        if (c.x < -c.w)
            out[1]++;
        if (c.y > c.w)
            out[2]++;
        if (c.y < -c.w)
            out[3]++;
        if (c.z > c.w)
            out[4]++;
        if (c.z < -c.w)
            out[5]++;
    }

    for (int i = 0; i < 6; i++)
    {
        if (out[i] == 8)
            return false;
    }
    return true;
}

//--------------------------------------------------------------------------
void Scene::Draw()
{
    if (!camera)
        return;
    renderedObjectCount = 0;

    // 1. ОПТИМИЗАЦИЯ МАТРИЦ
    glm::mat4 PV = camera->GetProjectionMatrix() * camera->GetViewMatrix();

    // --- ИСПРАВЛЕНИЕ: Транспонируем матрицу, чтобы получить доступ к строкам! ---
    glm::mat4 PVt = glm::transpose(PV);

    // 2. БЫСТРЫЙ FRUSTUM: Извлекаем 6 плоскостей из СТРОК матрицы
    glm::vec4 planes[6];
    planes[0] = PVt[3] + PVt[0]; // Left
    planes[1] = PVt[3] - PVt[0]; // Right
    planes[2] = PVt[3] + PVt[1]; // Bottom
    planes[3] = PVt[3] - PVt[1]; // Top
    planes[4] = PVt[3] + PVt[2]; // Near
    planes[5] = PVt[3] - PVt[2]; // Far

    // Нормализуем плоскости
    for (int i = 0; i < 6; i++)
    {
        float length = glm::length(glm::vec3(planes[i]));
        planes[i] /= length; // Нормализуем вектор нормали и дистанцию
    }

    glm::vec3 camPos = camera->GetPosition();

    // 3. ГЛАВНЫЙ ЦИКЛ ОТСЕЧЕНИЯ
    for (auto &obj : graphicObjects)
    {
        glm::vec3 pos = obj.GetPosition();
        float dist = glm::distance(camPos, pos);

        // --- УРОВНИ ДЕТАЛИЗАЦИИ (LoD) ---
        bool lodPassed = true;
        switch (obj.GetType())
        {
        case GraphicObjectType::road:
        case GraphicObjectType::building:
            lodPassed = true;
            break;
        case GraphicObjectType::vehicle:
            lodPassed = (dist <= 600.0f);
            break;
        case GraphicObjectType::big_nature:
            lodPassed = (dist <= 400.0f);
            break;
        case GraphicObjectType::big_prop:
            lodPassed = (dist <= 400.0f);
            break;
        case GraphicObjectType::medium_prop:
            lodPassed = (dist <= 300.0f);
            break;
        case GraphicObjectType::small_nature:
        case GraphicObjectType::small_prop:
            lodPassed = (dist <= 150.0f);
            break;
        default:
            lodPassed = true;
            break;
        }

        if (!lodPassed)
            continue;

        // --- FRUSTUM CULLING (Быстрая проверка сферы) ---
        float radius = glm::length(obj.GetDimensions()) / 2.0f;

        bool inFrustum = true;
        for (int i = 0; i < 6; i++)
        {
            if (glm::dot(glm::vec3(planes[i]), pos) + planes[i].w < -radius)
            {
                inFrustum = false;
                break;
            }
        }

        if (!inFrustum)
            continue;

        // Объект прошел все тесты! Отправляем на отрисовку
        renderedObjectCount++;
        RenderManager::instance().AddToRenderQueue(obj);
    }
}

//--------------------------------------------------------------------------
std::string Scene::GetSceneDescription()
{
    char buf[128];
    sprintf(buf, "%d/%zu objects rendered", renderedObjectCount, graphicObjects.size());
    return std::string(buf);
}

//--------------------------------------------------------------------------