#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <functional>

#include <QtOpenGL/QGL>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <core/types.h>
#include <utils/tree.h>
#include <utils/transform.h>
#include <utils/boundingsphere.h>
#include <utils/enumclass.h>
#include <utils/noncopyble.h>

#include "resourcestorage.h"

class QOpenGLExtraFunctions;
class ResourceStorage;
class Drawable;
struct BoundingSphere;
struct Frustum;

ENUMCLASS(LayerId, uint32_t,
          Selection,
          SolidGeometry,
          TransparencyGeometry)

struct RenderProgram : public ResourceStorage::Object
{
    GLuint id;
    RenderProgram(GLuint id_) : id(id_) {}
    ~RenderProgram() override;

    void setupTransformFeedback(const std::vector<std::string>&, GLenum);

    GLuint uniformBufferIndexByName(const std::string&);
    GLint uniformBufferDataSize(GLuint);
    std::unordered_map<std::string, GLint> uniformBufferOffsets(GLuint); // it works but don't use it better

    GLint uniformLocation(const std::string&);

    void setUniform(GLint, GLint);
    void setUniform(GLint, const glm::vec4&);
    void setUniform(GLint, const glm::mat4x4&);
};

struct Texture : public ResourceStorage::Object
{
    GLenum target;
    GLuint id;

    Texture(GLuint id_) : target(GL_TEXTURE_2D), id(id_) {}
    ~Texture() override;

    void generateMipmaps();
};

struct Buffer
{
    NONCOPYBLE(Buffer)

    GLuint id;

    Buffer(GLsizeiptr, GLvoid*, GLenum);
    virtual ~Buffer();

    void *map(GLintptr, GLsizeiptr, GLbitfield);
    void unmap();
};

struct VertexBuffer : public Buffer
{
    uint32_t numVertices;
    uint32_t numComponents;

    VertexBuffer(uint32_t, uint32_t, float*, GLenum);
};

struct IndexBuffer : public Buffer
{
    uint32_t numIndices;
    GLenum primitiveType;

    IndexBuffer(GLenum, uint32_t, uint32_t*, GLenum);
};

struct Mesh
{
    NONCOPYBLE(Mesh)

    GLuint id;
    std::unordered_map<VertexAttribute, std::shared_ptr<VertexBuffer>> attributesDeclaration;
    std::unordered_set<std::shared_ptr<IndexBuffer>> indexBuffers;
    BoundingSphere boundingSphere;

    Mesh();
    ~Mesh();

    void declareVertexAttribute(VertexAttribute, std::shared_ptr<VertexBuffer>);
    void undeclareVertexAttribute(VertexAttribute);
    std::shared_ptr<VertexBuffer> vertexBuffer(VertexAttribute) const;

    void attachIndexBuffer(std::shared_ptr<IndexBuffer>);
};

struct Framebuffer
{
    NONCOPYBLE(Framebuffer)

    GLuint id;
    GLuint colorTexture;
    GLuint depthTexture;
    GLint colorTextureInternalFormat;

    Framebuffer(GLint);
    ~Framebuffer();

    void resize(int, int);
};

struct Model : public ResourceStorage::Object
{
    struct Material;
    struct Mesh;
    struct Animation;
    struct Node;

    std::shared_ptr<Node> rootNode;
    std::unordered_map<std::string, std::shared_ptr<Animation>> animations;
    std::vector<Transform> boneTransforms;
    std::vector<std::string> boneNames;

    uint32_t numBones() const;
    bool calcBoneTransforms(const std::string&, float, std::vector<Transform>&) const;
};

struct Model::Material
{
    std::pair<std::string, std::shared_ptr<Texture>> diffuseTexture;
};

struct Model::Mesh
{
    std::shared_ptr<::Mesh> mesh;
    std::shared_ptr<Material> material;

    Mesh(std::shared_ptr<::Mesh> msh, std::shared_ptr<Material> mtl)
        : mesh(msh)
        , material(mtl)
    {}
};

struct Model::Animation : public ResourceStorage::Object
{
    float framesPerSecond;
    float duration;

    std::unordered_map<std::string, std::tuple<
        std::vector<std::pair<float, glm::vec3>>,
        std::vector<std::pair<float, glm::quat>>,
        std::vector<std::pair<float, glm::vec3>>
    >> transforms;

    Animation(float fps, float d)
        : framesPerSecond(fps)
        , duration(d)
    {
    }
};

struct Model::Node : public TreeNode<Node>
{
    Transform transform;
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::int32_t boneIndex;

    Node(const Transform& t = Transform())
        : transform(t)
        , boneIndex(-1)
    {
    }
};

class Renderer
{
    NONCOPYBLE(Renderer)

public:
    Renderer(QOpenGLExtraFunctions&);
    void initializeResources();

    static Renderer& instance();
    QOpenGLExtraFunctions& functions();

    std::shared_ptr<RenderProgram> loadRenderProgram(const std::string&, const std::string&);
    std::shared_ptr<Texture> loadTexture(const std::string&);
    std::shared_ptr<Model> loadModel(const std::string&);
    std::shared_ptr<Model::Animation> loadAnimation(const std::string&);

    // binding
    void bindTexture(std::shared_ptr<Texture>, GLint);
    void bindUniformBuffer(std::shared_ptr<Buffer>, GLuint);

    void draw(std::shared_ptr<Drawable>, const Transform&);
    void pick(int, int, const glm::vec4&, uint32_t&, float&);

    void setViewMatrix(const glm::mat4x4&);
    void setProjectionMatrix(float, float, float);

    const glm::mat4x4& projectionMatrix() const;
    const glm::ivec4& viewport() const;

private:
    using DrawDataType = std::pair<std::shared_ptr<Drawable>, Transform>;
    struct DrawDataComarator
    {
        using is_transparent = void;
        bool operator ()(const DrawDataType&, const DrawDataType&) const;
        bool operator ()(const DrawDataType&, LayerId) const;
    };
    using DrawDataContainer = std::multiset<DrawDataType, DrawDataComarator>;

    void resize(int, int);
    void render();
    void renderSelectionLayer(DrawDataContainer::iterator, DrawDataContainer::iterator);
    void renderSolidLayer(DrawDataContainer::iterator, DrawDataContainer::iterator);
    void renderTransparentLayer(DrawDataContainer::iterator, DrawDataContainer::iterator);

    QOpenGLExtraFunctions& m_functions;
    std::unique_ptr<ResourceStorage> m_resourceStorage;
    std::shared_ptr<Framebuffer> m_selectionFramebuffer;
    DrawDataContainer m_drawData;
    glm::mat4x4 m_projMatrix;
    glm::mat4x4 m_viewMatrix;
    glm::ivec4 m_viewport;
    float m_fov = glm::pi<float>() * 0.25f, m_zNear = 0.5f, m_zFar = 10000.0f;

    friend class RenderWidget;
};

#endif // RENDERER_H
