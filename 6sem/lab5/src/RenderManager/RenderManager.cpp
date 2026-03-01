#include "RenderManager.h"

//*--------------------------------------------------------------------------
#include "../ResourceManager/ResourceManager.h"
#include "../Texture/Texture.h"
#include <GL/glew.h>

//*--------------------------------------------------------------------------
void RenderManager::Init()
{
    // Выделяем место под 1 шейдер в векторе
    shaders.resize(1);

    // Загружаем файлы напрямую в этот элемент
    shaders[0].Load("../src/SHADERS/Example.vsh", "../src/SHADERS/Example.fsh");
}

//*--------------------------------------------------------------------------
void RenderManager::Start()
{
    graphicObjects.clear();
}

//*--------------------------------------------------------------------------
void RenderManager::SetCamera(Camera *cam)
{
    camera = cam;
}

//*--------------------------------------------------------------------------
void RenderManager::AddToRenderQueue(const GraphicObject &graphicObject)
{
    graphicObjects.push_back(graphicObject);
}

//*--------------------------------------------------------------------------
void RenderManager::Finish()
{
    // очистка буфера кадра
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if (shaders.empty() || camera == nullptr)
        return;

    // активируем шейдер
    shaders[0].Activate(); // Или используй тот метод, что у тебя в классе Shader для glUseProgram

    glm::mat4 projectionMatrix = camera->GetProjectionMatrix();
    shaders[0].SetUniform("projectionMatrix", projectionMatrix);

    glm::mat4 viewMatrix = camera->GetViewMatrix();

    for (auto &obj : graphicObjects)
    {
        glm::mat4 modelViewMatrix = viewMatrix * obj.GetModelMatrix();
        shaders[0].SetUniform("modelViewMatrix", modelViewMatrix);
        shaders[0].SetUniform("color", obj.GetColor());

        // Привязываем текстуру
        int texID = obj.GetTextureId();
        Texture *texture = ResourceManager::instance().GetTexture(texID);
        if (texture != nullptr)
        {
            texture->Bind(GL_TEXTURE0);
        }
        // shaders[0].SetUniform("texture_0", 0); // 0 - это GL_TEXTURE0

        // Рисуем меш
        int meshID = obj.GetMeshId();
        Mesh *mesh = ResourceManager::instance().GetMesh(meshID);
        if (mesh != nullptr)
        {
            mesh->DrawOne();
        }
    }
}

//*--------------------------------------------------------------------------