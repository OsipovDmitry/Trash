#ifndef RENDERER_H
#define RENDERER_H

#include <string>

#include <QtOpenGL/QGL>

#include "resourcestorage.h"

class QOpenGLExtraFunctions;

struct RenderProgram : public ResourceStorage::Object
{
    GLuint id;
    RenderProgram(GLuint id_) : id(id_) {}
};

struct Texture : public ResourceStorage::Object
{
    GLuint id;
    Texture(GLuint id_) : id(id_) {}
};

class Renderer
{
public:
    Renderer(QOpenGLExtraFunctions&);
    ~Renderer();

    std::shared_ptr<RenderProgram> loadRenderProgram(const std::string&, const std::string&);
    std::shared_ptr<Texture> loadTexture(const std::string&);

private:
    void resize(int, int);
    void render();

    QOpenGLExtraFunctions& m_functions;

    std::shared_ptr<RenderProgram> m_renderProgram;
    std::shared_ptr<Texture> m_texture;
    GLuint m_vao = 0, m_vbo = 0;

    friend class RenderWidget;
};

#endif // RENDERER_H
