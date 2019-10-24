#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <unordered_map>
#include <functional>

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

    virtual GLuint vao() const { return 0; }
    virtual uint32_t numIndices() const { return 0; }
};

struct Model : public ResourceStorage::Object
{
    struct Material;
    struct Mesh;
    struct Node;

    std::shared_ptr<Node> rootNode;
};

struct Model::Material
{
    std::shared_ptr<Texture> diffuseTexture;
};

struct Model::Mesh
{
    GLuint vao, vbo, ibo;
    uint32_t numIndices;
    VertexDeclaration vertexDeclaration;
    std::shared_ptr<Material> material;

    Mesh(GLuint vao_, GLuint vbo_, GLuint ibo_, uint32_t numIndices_, const VertexDeclaration& vertexDeclaration_)
        : vao(vao_)
        , vbo(vbo_)
        , ibo(ibo_)
        , numIndices(numIndices_)
        , vertexDeclaration(vertexDeclaration_)
    {}
};

struct Model::Node : public TreeNode<Node>
{
    Transform transform;
    std::vector<std::shared_ptr<Mesh>> meshes;

    Node(const Transform& t)
        : transform(t)
    {
    }
};

class Renderer
{
public:
    Renderer(QOpenGLExtraFunctions&);
    ~Renderer();

    std::shared_ptr<RenderProgram> loadRenderProgram(const std::string&, const std::string&);
    std::shared_ptr<Texture> loadTexture(const std::string&);
    std::shared_ptr<Model> loadModel(const std::string&);

    // textures
    void generateMipmaps(std::shared_ptr<Texture>);

    void draw(uint32_t, std::shared_ptr<Drawable>, const Transform&);

private:
    using DrawDataType = std::unordered_multimap<uint32_t, std::pair<std::shared_ptr<Drawable>, Transform>>;

    void resize(int, int);
    void render();
    void renderLayer(DrawDataType::iterator, DrawDataType::iterator);

    QOpenGLExtraFunctions& m_functions;
    glm::mat4x4 m_projMatrix;
    DrawDataType m_drawData;
    std::shared_ptr<Texture> m_standardTexture;

    std::function<GLuint(const std::string&, GLenum, std::string&)> m_loadShader;
    std::function<void(GLuint)> m_deleteShader;
    std::function<GLuint(GLuint, GLuint, std::string&)> m_loadProgram;
    std::function<void(GLuint)> m_deleteProgram;
    std::function<void(RenderProgram*)> m_renderProgramDeleter;
    std::function<void(Texture*)> m_textureDeleter;
    std::function<void(Model::Mesh*)> m_meshDeleter;

    friend class RenderWidget;
};

#endif // RENDERER_H
