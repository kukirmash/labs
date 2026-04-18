#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

//*--------------------------------------------------------------------------
#include <vector>
#include "../Shader/Shader.h"
#include "../Camera/Camera.h"
#include "../GraphicObject/GraphicObject.h"

//*--------------------------------------------------------------------------
class RenderManager
{
    std::vector<Shader> shaders;
    Camera *camera;
    std::vector<GraphicObject> graphicObjects;

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
};

//*--------------------------------------------------------------------------
#endif // RENDER_MANAGER_H