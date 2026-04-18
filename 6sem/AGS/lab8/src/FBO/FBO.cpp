#include "FBO.h"
#include <iostream>

//--------------------------------------------------------------------------
FBO::FBO() : FBO_Index(0), colorTex(0), depthTex(0), width(0), height(0), samples(0)
{
}

//--------------------------------------------------------------------------
void FBO::init(int w, int h, bool multiSamples)
{
    width = w;
    height = h;

    // Создаем сам буфер FBO
    glGenFramebuffers(1, &FBO_Index);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO_Index);

    glGenTextures(1, &colorTex);
    glGenTextures(1, &depthTex);

    if (multiSamples)
    {
        // Узнаем максимальное количество выборок поддерживаемое видеокартой
        glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &samples);
        if (samples > 8)
            samples = 8; // Ограничим до 8 для надежности

        // Текстура цвета MSAA
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorTex);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, width, height, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, colorTex, 0);

        // Текстура глубины MSAA
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTex);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH_COMPONENT24, width, height, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthTex, 0);
    }
    else
    {
        // Обычная текстура цвета
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        // Устанавливаем параметры фильтрации (без мипмапов)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

        // Обычная текстура глубины
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    }

    // Проверка корректности создания FBO
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "FBO creation failed!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//--------------------------------------------------------------------------
void FBO::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO_Index);
}

//--------------------------------------------------------------------------
void FBO::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//--------------------------------------------------------------------------
void FBO::clear()
{
    if (FBO_Index)
        glDeleteFramebuffers(1, &FBO_Index);
    if (colorTex)
        glDeleteTextures(1, &colorTex);
    if (depthTex)
        glDeleteTextures(1, &depthTex);
    FBO_Index = colorTex = depthTex = 0;
}

//--------------------------------------------------------------------------
void FBO::resolveToFbo(FBO &fbo)
{
    // Копирование (Blitting) мультисэмпл-буфера в обычный
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO_Index);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo.FBO_Index);
    glBlitFramebuffer(0, 0, width, height, 0, 0, fbo.width, fbo.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

//--------------------------------------------------------------------------
void FBO::bindColorTexture(GLenum texUnit)
{
    glActiveTexture(texUnit);
    glBindTexture(GL_TEXTURE_2D, colorTex);
}

//--------------------------------------------------------------------------
void FBO::bindDepthTexture(GLenum texUnit)
{
    glActiveTexture(texUnit);
    glBindTexture(GL_TEXTURE_2D, depthTex);
}

//--------------------------------------------------------------------------