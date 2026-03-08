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
    shaders[0].Load("../src/SHADERS/DiffuseTexture.vsh", "../src/SHADERS/DiffuseTexture.fsh");
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
    // Очищаем экран голубым цветом
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if (shaders.empty() || camera == nullptr)
        return;

    shaders[0].Activate();

    glm::mat4 projectionMatrix = camera->GetProjectionMatrix();
    glm::mat4 viewMatrix = camera->GetViewMatrix();

    // Передаем матрицу проекции
    shaders[0].SetUniform("projectionMatrix", projectionMatrix);

    // Передаем параметры света (имена точно как в DiffuseTexture.fsh!)
    shaders[0].SetUniform("lPosition", mainLight.position);
    shaders[0].SetUniform("lAmbient", mainLight.ambient);
    shaders[0].SetUniform("lDiffuse", mainLight.diffuse);
    shaders[0].SetUniform("lSpecular", mainLight.specular);

    for (auto &obj : graphicObjects)
    {
        // ВАЖНО: Шейдер просит modelViewMatrix (View * Model)
        glm::mat4 modelMatrix = obj.GetModelMatrix();
        glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
        shaders[0].SetUniform("modelViewMatrix", modelViewMatrix);

        // Настраиваем материал (имена точно как в DiffuseTexture.fsh!)
        Material *mat = ResourceManager::instance().GetMaterial(obj.GetMaterialId());
        if (mat != nullptr)
        {
            shaders[0].SetUniform("mAmbient", mat->ambient);
            shaders[0].SetUniform("mDiffuse", mat->diffuse);
            shaders[0].SetUniform("mSpecular", mat->specular);
            shaders[0].SetUniform("mShininess", mat->shininess);
        }

        // Привязываем текстуру
        Texture *texture = ResourceManager::instance().GetTexture(obj.GetTextureId());
        if (texture != nullptr)
            texture->Bind(GL_TEXTURE0);

        // Рисуем меш
        Mesh *mesh = ResourceManager::instance().GetMesh(obj.GetMeshId());
        if (mesh != nullptr)
            mesh->DrawOne();
    }
}

//*--------------------------------------------------------------------------