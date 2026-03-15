#include "RenderManager.h"

//*--------------------------------------------------------------------------
#include "../ResourceManager/ResourceManager.h"
#include "../Texture/Texture.h"
#include <GL/glew.h>
#include <algorithm>

//*--------------------------------------------------------------------------
void RenderManager::Init()
{
    shaders.resize(1);

    shaders[0].Load("../src/DATA/SHADERS/DiffuseTextureInstanced.vsh", "../src/DATA/SHADERS/DiffuseTextureInstanced.fsh");
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
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if (shaders.empty() || camera == nullptr)
        return;

    // 1. Сброс статистики перед отрисовкой кадра
    materialChangeCount = 0;
    textureChangeCount = 0;
    drawCallCount = 0;

    // 2. Сортировка по Material -> Texture -> Mesh
    std::sort(graphicObjects.begin(), graphicObjects.end(), [](const GraphicObject &a, const GraphicObject &b)
              {
        if (a.GetMaterialId() != b.GetMaterialId()) return a.GetMaterialId() < b.GetMaterialId();
        if (a.GetTextureId() != b.GetTextureId()) return a.GetTextureId() < b.GetTextureId();
        return a.GetMeshId() < b.GetMeshId(); });

    shaders[0].Activate();

    glm::mat4 projectionMatrix = camera->GetProjectionMatrix();
    glm::mat4 viewMatrix = camera->GetViewMatrix();
    shaders[0].SetUniform("projectionMatrix", projectionMatrix);

    // Передаем параметры света
    shaders[0].SetUniform("lPosition", mainLight.position);
    shaders[0].SetUniform("lAmbient", mainLight.ambient);
    shaders[0].SetUniform("lDiffuse", mainLight.diffuse);
    shaders[0].SetUniform("lSpecular", mainLight.specular);

    int currentMaterial = -1;
    int currentTexture = -1;
    int currentMesh = -1;

    std::vector<glm::mat4> batchMatrices;

    // 3. Функция (лямбда) для отрисовки накопленной пачки объектов
    auto flushBatch = [&]()
    {
        if (!batchMatrices.empty() && currentMesh != -1)
        {
            Mesh *mesh = ResourceManager::instance().GetMesh(currentMesh);
            if (mesh != nullptr)
            {
                // Финальный щит: отсекаем излишки
                if (batchMatrices.size() > 20)
                    batchMatrices.resize(20);

                shaders[0].SetUniform("modelViewMatrix", batchMatrices);
                mesh->DrawMany(batchMatrices.size());
                drawCallCount++;
            }
        }
        batchMatrices.clear();
    };

    // 4. Главный цикл Instancing
    for (auto &obj : graphicObjects)
    {
        // Проверяем, нужно ли прервать пачку (лимит 5 штук или сменился ресурс)
        bool needFlush = (batchMatrices.size() >= 20) ||
                         (!batchMatrices.empty() &&
                          (obj.GetMaterialId() != currentMaterial ||
                           obj.GetTextureId() != currentTexture ||
                           obj.GetMeshId() != currentMesh));

        if (needFlush)
        {
            flushBatch();
        }

        // Устанавливаем новый материал, если он изменился
        if (obj.GetMaterialId() != currentMaterial)
        {
            currentMaterial = obj.GetMaterialId();
            materialChangeCount++; // Статистика
            Material *mat = ResourceManager::instance().GetMaterial(currentMaterial);
            if (mat)
            {
                shaders[0].SetUniform("mAmbient", mat->ambient);
                shaders[0].SetUniform("mDiffuse", mat->diffuse);
                shaders[0].SetUniform("mSpecular", mat->specular);
                shaders[0].SetUniform("mShininess", mat->shininess);
            }
        }

        // Устанавливаем новую текстуру, если она изменилась
        if (obj.GetTextureId() != currentTexture)
        {
            currentTexture = obj.GetTextureId();
            textureChangeCount++; // Статистика
            Texture *tex = ResourceManager::instance().GetTexture(currentTexture);
            if (tex)
                tex->Bind(GL_TEXTURE0);
        }

        currentMesh = obj.GetMeshId();

        // Накапливаем матрицы (сдвигаем объект в пространство вида камеры)
        glm::mat4 modelViewMatrix = viewMatrix * obj.GetModelMatrix();
        batchMatrices.push_back(modelViewMatrix);
    }

    // 5. Обязательно выводим "хвост" - оставшиеся матрицы после завершения цикла
    flushBatch();
}

//*--------------------------------------------------------------------------