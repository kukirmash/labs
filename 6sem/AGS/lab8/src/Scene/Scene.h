#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include <rapidjson/document.h>
#include "../GraphicObject/GraphicObject.h"
#include "../Camera/Camera.h"
#include "../Light/Light.h"

//--------------------------------------------------------------------------
// КЛАСС ДЛЯ РАБОТЫ СО СЦЕНОЙ
class Scene
{
    GraphicObject CreateGraphicObject(std::string model);
    bool LodTest(GraphicObject &graphicObject);
    bool FrustumCullingTest(GraphicObject &graphicObject);

    rapidjson::Document document;
    Camera *camera;
    Light *light;

    std::vector<GraphicObject> graphicObjects;
    int renderedObjectCount;

public:
    Scene() : camera(nullptr), light(nullptr), renderedObjectCount(0) {}

    void Init(std::string filename);
    bool LoadFromJson(std::string filename);

    void SetCamera(Camera *camera);
    void SetLight(Light *light);

    void Draw();
    std::string GetSceneDescription();
};

//--------------------------------------------------------------------------
#endif