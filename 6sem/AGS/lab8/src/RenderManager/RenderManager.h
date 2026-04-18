#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

//*--------------------------------------------------------------------------
#include <vector>
#include "../Shader/Shader.h"
#include "../Camera/Camera.h"
#include "../GraphicObject/GraphicObject.h"
#include "../Light/Light.h"
#include "../FBO/FBO.h"

//*--------------------------------------------------------------------------
class RenderManager
{
    std::vector<Shader> shaders;
    Camera *camera;
    std::vector<GraphicObject> graphicObjects;

    Light mainLight;

    int materialChangeCount = 0;
    int textureChangeCount = 0;
    int drawCallCount = 0;

    FBO fbos[2];                // fbos[0] - для MSAA, fbos[1] - для обычной текстуры
    GLuint quadVAO, quadVBO;    // Для вывода прямоугольника на экран
    int postProcessingMode = 0; // Текущий режим (0 - нет, 1 - ч/б, 2 - сепия)

    RenderManager() {}
    RenderManager(const RenderManager &) = delete;
    RenderManager &operator=(const RenderManager &) = delete;

public:
    static RenderManager &instance()
    {
        static RenderManager rm;
        return rm;
    }

    void Init();
    void Start();
    void SetCamera(Camera *cam);
    void AddToRenderQueue(const GraphicObject &graphicObject);
    void Finish();
    void ResizeFBOs(int w, int h);

    void TogglePostProcessMode(); // Переключение режимов
    int GetPostProcessMode() const { return postProcessingMode; }

    int GetMaterialChangeCount() const
    {
        return materialChangeCount;
    }
    int GetTextureChangeCount() const
    {
        return textureChangeCount;
    }
    int GetDrawCallCount() const
    {
        return drawCallCount;
    }
};

//*--------------------------------------------------------------------------
#endif // RENDER_MANAGER_H