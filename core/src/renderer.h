#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <unordered_set>

#include <QtOpenGL/QGL>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <core/types.h>
#include <utils/tree.h>
#include <utils/transform.h>
#include <utils/enumclass.h>

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

class Drawable
{
public:
    virtual ~Drawable() = default;
    virtual std::shared_ptr<RenderProgram> renderProgram() const { return nullptr; }
    virtual std::shared_ptr<Texture> diffuseTexture() const { return nullptr; }
    virtual glm::mat4x4 modelMatrix() const { return glm::mat4x4(1.0f); }

    virtual GLuint vao() const { return 0; }
    virtual uint32_t numIndices() const { return 0; }
};

class Renderer
{
public:
    Renderer(QOpenGLExtraFunctions&);
    ~Renderer();

    std::shared_ptr<RenderProgram> loadRenderProgram(const std::string&, const std::string&);
    std::shared_ptr<Texture> loadTexture(const std::string&);

    unsigned int generateTriangle();

    void draw(std::shared_ptr<Drawable>);

private:
    void resize(int, int);
    void render();

    QOpenGLExtraFunctions& m_functions;
    std::unordered_set<std::shared_ptr<Drawable>> m_drawData;

    friend class RenderWidget;
};

#endif // RENDERER_H
