#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

//*--------------------------------------------------------------------------
#include <vector>
#include "../Shader/Shader.h"
#include "../Camera/Camera.h"
#include "../GraphicObject/GraphicObject.h"
#include "../Light/Light.h"

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