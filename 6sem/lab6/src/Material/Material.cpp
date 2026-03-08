#include "Material.h"
#include <iostream>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

//--------------------------------------------------------------------------
bool Material::LoadFromJson(const std::string &filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open())
        return false;

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);

    if (doc.HasParseError())
        return false;

    // Читаем все 4 компонента (RGBA) для каждой характеристики
    if (doc.HasMember("ambient") && doc["ambient"].IsArray())
    {
        ambient.r = doc["ambient"][0].GetFloat();
        ambient.g = doc["ambient"][1].GetFloat();
        ambient.b = doc["ambient"][2].GetFloat();
        ambient.a = doc["ambient"][3].GetFloat(); // Читаем 4-й элемент
    }
    if (doc.HasMember("diffuse") && doc["diffuse"].IsArray())
    {
        diffuse.r = doc["diffuse"][0].GetFloat();
        diffuse.g = doc["diffuse"][1].GetFloat();
        diffuse.b = doc["diffuse"][2].GetFloat();
        diffuse.a = doc["diffuse"][3].GetFloat(); // Читаем 4-й элемент
    }
    if (doc.HasMember("specular") && doc["specular"].IsArray())
    {
        specular.r = doc["specular"][0].GetFloat();
        specular.g = doc["specular"][1].GetFloat();
        specular.b = doc["specular"][2].GetFloat();
        specular.a = doc["specular"][3].GetFloat(); // Читаем 4-й элемент
    }
    if (doc.HasMember("shininess") && doc["shininess"].IsNumber())
    {
        shininess = doc["shininess"].GetFloat();
    }

    return true;
}

//--------------------------------------------------------------------------