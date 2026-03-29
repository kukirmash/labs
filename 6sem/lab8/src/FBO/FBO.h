#ifndef FBO_H
#define FBO_H

#include <GL/glew.h>

//*--------------------------------------------------------------------------
class FBO
{
    GLuint FBO_Index;
    GLuint colorTex;
    GLuint depthTex;
    int width, height, samples;

public:
    FBO();
    // инициализация FBO (создание самого буфера, текстуры цвета и глубины)
    void init(int w, int h, bool multiSamples = false);

    // выбор FBO в качестве текущего для рендеринга
    void bind();

    // возврат к буферу кадра по умолчанию (для вывода на экран)
    static void unbind();

    void clear();

    // копирование текущего FBO в другой FBO
    void resolveToFbo(FBO &fbo);

    // привязать текстуру цвета к нужному текстурному блоку
    void bindColorTexture(GLenum texUnit = GL_TEXTURE0);

    // привязать текстуру глубины к нужному текстурному блоку
    void bindDepthTexture(GLenum texUnit = GL_TEXTURE1);
};

//*--------------------------------------------------------------------------
#endif