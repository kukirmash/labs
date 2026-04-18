#include "RenderManager.h"

//--------------------------------------------------------------------------
#include "../ResourceManager/ResourceManager.h"
#include "../Texture/Texture.h"
#include <GL/glew.h>
#include <algorithm>

//--------------------------------------------------------------------------
void RenderManager::Start()
{
    graphicObjects.clear();
}

//--------------------------------------------------------------------------
void RenderManager::SetCamera(Camera *cam)
{
    camera = cam;
}

//--------------------------------------------------------------------------
void RenderManager::AddToRenderQueue(const GraphicObject &graphicObject)
{
    graphicObjects.push_back(graphicObject);
}

//--------------------------------------------------------------------------
void RenderManager::Init()
{
    shaders.resize(4); // Нам нужно 4 шейдера

    // 0. Основной шейдер для отрисовки самой сцены
    shaders[0].Load("../src/DATA/SHADERS/DiffuseTextureInstanced.vsh", "../src/DATA/SHADERS/DiffuseTextureInstanced.fsh");

    // 1. Шейдер обычного вывода (без эффектов)
    shaders[1].Load("../src/DATA/SHADERS/SimplePostProcessing.vsh", "../src/DATA/SHADERS/SimplePostProcessing.fsh");

    // 2. Шейдер черно-белого фильтра
    shaders[2].Load("../src/DATA/SHADERS/GreyPostProcessing.vsh", "../src/DATA/SHADERS/GreyPostProcessing.fsh");

    // 3. Шейдер сепии
    shaders[3].Load("../src/DATA/SHADERS/SepiaPostProcessing.vsh", "../src/DATA/SHADERS/SepiaPostProcessing.fsh");

    // Инициализация FBO (размер должен совпадать с размером окна glutInitWindowSize)
    fbos[0].init(800, 800, true);  // Буфер 0: С мультисэмплингом (для MSAA)
    fbos[1].init(800, 800, false); // Буфер 1: Обычный, для натяжки на экран

    // Создаем прямоугольник (Quad), на который будет натянута картинка сцены
    // Координаты от -0.5 до 0.5, так как в твоих шейдерах они умножаются на 2.0
    float quadVertices[] = {
        -0.5f, 0.5f,
        -0.5f, -0.5f,
        0.5f, 0.5f,
        0.5f, -0.5f};
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    // Обрати внимание: теперь шаг данных 2 * sizeof(float), так как мы передаем только X и Y
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glBindVertexArray(0);
}

//--------------------------------------------------------------------------
// 2. Добавляем метод переключения:
void RenderManager::TogglePostProcessMode()
{
    postProcessingMode++;
    if (postProcessingMode > 2)
        postProcessingMode = 0;
}

//--------------------------------------------------------------------------
void RenderManager::ResizeFBOs(int w, int h)
{
    if (w <= 0 || h <= 0)
        return;

    // Удаляем старые буферы размером 800x800 (или предыдущие)
    fbos[0].clear();
    fbos[1].clear();

    // Создаем новые буферы с актуальным размером окна
    fbos[0].init(w, h, true);  // MSAA буфер
    fbos[1].init(w, h, false); // Обычный буфер
}

//--------------------------------------------------------------------------
// 3. Полностью меняем метод Finish() на двухпроходный [cite: 1136-1160]:
void RenderManager::Finish()
{
    if (shaders.empty() || camera == nullptr)
        return;

    // --- ПЕРВЫЙ ПРОХОД: Рисуем сцену в FBO ---
    fbos[0].bind();

    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE); // Включаем мультисэмплинг [cite: 1094]

    materialChangeCount = 0;
    textureChangeCount = 0;
    drawCallCount = 0;

    std::sort(graphicObjects.begin(), graphicObjects.end(), [](const GraphicObject &a, const GraphicObject &b)
              {
        if (a.GetMaterialId() != b.GetMaterialId()) return a.GetMaterialId() < b.GetMaterialId();
        if (a.GetTextureId() != b.GetTextureId()) return a.GetTextureId() < b.GetTextureId();
        return a.GetMeshId() < b.GetMeshId(); });

    shaders[0].Activate();
    shaders[0].SetUniform("projectionMatrix", camera->GetProjectionMatrix());
    shaders[0].SetUniform("lPosition", mainLight.position);
    shaders[0].SetUniform("lAmbient", mainLight.ambient);
    shaders[0].SetUniform("lDiffuse", mainLight.diffuse);
    shaders[0].SetUniform("lSpecular", mainLight.specular);

    int currentMaterial = -1, currentTexture = -1, currentMesh = -1;
    std::vector<glm::mat4> batchMatrices;

    auto flushBatch = [&]()
    {
        if (!batchMatrices.empty() && currentMesh != -1)
        {
            Mesh *mesh = ResourceManager::instance().GetMesh(currentMesh);
            if (mesh != nullptr)
            {
                if (batchMatrices.size() > 20)
                    batchMatrices.resize(20);
                shaders[0].SetUniform("modelViewMatrix", batchMatrices);
                mesh->DrawMany(batchMatrices.size());
                drawCallCount++;
            }
        }
        batchMatrices.clear();
    };

    for (auto &obj : graphicObjects)
    {
        bool needFlush = (batchMatrices.size() >= 20) ||
                         (!batchMatrices.empty() &&
                          (obj.GetMaterialId() != currentMaterial ||
                           obj.GetTextureId() != currentTexture ||
                           obj.GetMeshId() != currentMesh));

        if (needFlush)
            flushBatch();

        if (obj.GetMaterialId() != currentMaterial)
        {
            currentMaterial = obj.GetMaterialId();
            materialChangeCount++;
            Material *mat = ResourceManager::instance().GetMaterial(currentMaterial);
            if (mat)
            {
                shaders[0].SetUniform("mAmbient", mat->ambient);
                shaders[0].SetUniform("mDiffuse", mat->diffuse);
                shaders[0].SetUniform("mSpecular", mat->specular);
                shaders[0].SetUniform("mShininess", mat->shininess);
            }
        }

        if (obj.GetTextureId() != currentTexture)
        {
            currentTexture = obj.GetTextureId();
            textureChangeCount++;
            Texture *tex = ResourceManager::instance().GetTexture(currentTexture);
            if (tex)
                tex->Bind(GL_TEXTURE0);
        }

        currentMesh = obj.GetMeshId();
        batchMatrices.push_back(camera->GetViewMatrix() * obj.GetModelMatrix());
    }
    flushBatch();

    // --- ВТОРОЙ ПРОХОД: Копируем MSAA -> Обычную текстуру и рисуем на экран ---
    fbos[0].resolveToFbo(fbos[1]);
    FBO::unbind(); // Возврат к экрану

    // Отключаем тесты, т.к. рисуем просто плоский экран
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Активируем нужный шейдер (0 - none, 1 - gray, 2 - sepia)
    shaders[postProcessingMode + 1].Activate();
    shaders[postProcessingMode + 1].SetUniform("texture_0", 0);

    fbos[1].bindColorTexture(GL_TEXTURE0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Рисуем экран
    glBindVertexArray(0);
}

//--------------------------------------------------------------------------