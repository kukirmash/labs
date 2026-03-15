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

        // Читаем габариты [cite: 63-66]
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
    renderedObjectCount = 0;
    for (auto &grobj : graphicObjects)
    {
        if (!LodTest(grobj))
            continue;
        if (!FrustumCullingTest(grobj))
            continue;

        renderedObjectCount++;
        RenderManager::instance().AddToRenderQueue(grobj);
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