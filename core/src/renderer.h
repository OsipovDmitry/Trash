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
#include <core/forwarddecl.h>
#include <utils/tree.h>
#include <utils/transform.h>
#include <utils/boundingsphere.h>
#include <utils/enumclass.h>
#include <utils/noncopyble.h>

#include "resourcestorage.h"

class QOpenGLExtraFunctions;

namespace trash
{
namespace core
{

class ResourceStorage;
class Drawable;

ENUMCLASS(LayerId, uint32_t,
          Selection,
          Shadows,
          SolidGeometry,
          TransparencyGeometry)

struct RenderProgram : public ResourceStorage::Object
{
    GLuint id;
    RenderProgram(GLuint id_) : id(id_) {}
    ~RenderProgram() override;

    void setupTransformFeedback(const std::vector<std::string>&, GLenum);

    GLuint uniformBufferIndexByName(const std::string&);
    void setUniformBufferBinding(GLuint, GLuint);
    GLint uniformBufferDataSize(GLuint);
    std::unordered_map<std::string, GLint> uniformBufferOffsets(GLuint); // it works but don't use it better

    GLint uniformLocation(const std::string&);

    void setUniform(GLint, GLint);
    void setUniform(GLint, const glm::vec3&);
    void setUniform(GLint, const glm::vec4&);
    void setUniform(GLint, const glm::mat3x3&);
    void setUniform(GLint, const glm::mat4x4&);
};

struct Texture : public ResourceStorage::Object
{
    GLenum target;
    GLuint id;

    Texture(GLuint id_, GLenum target_) : target(target_), id(id_) {}
    ~Texture() override;

    void generateMipmaps();
    void setFilter(int32_t); // 1 - nearest // 2 - linear // 3 - trilinear
    void setWrap(GLenum);
    void setCompareMode(GLenum);
    void setCompareFunc(GLenum);
    void setBorderColor(const glm::vec4&);
    int32_t numMipmapLevels() const;
};

struct Buffer
{
    NONCOPYBLE(Buffer)

    GLuint id;

    Buffer(GLsizeiptr, GLvoid*, GLenum);
    virtual ~Buffer();

    void setSubData(GLintptr, GLsizeiptr, const void*);

    void *map(GLintptr, GLsizeiptr, GLbitfield);
    static void unmap();
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
    utils::BoundingSphere boundingSphere;

    Mesh();
    ~Mesh();

    void declareVertexAttribute(VertexAttribute, std::shared_ptr<VertexBuffer>);
    void undeclareVertexAttribute(VertexAttribute);
    std::shared_ptr<VertexBuffer> vertexBuffer(VertexAttribute) const;

    void attachIndexBuffer(std::shared_ptr<IndexBuffer>);
};

struct Renderbuffer
{
    NONCOPYBLE(Renderbuffer)

    GLuint id;

    Renderbuffer(GLenum, GLsizei, GLsizei);
    ~Renderbuffer();
};

struct Framebuffer
{
    NONCOPYBLE(Framebuffer)

    GLuint id;
    std::shared_ptr<Texture> colorTexture, depthTexture;
    std::shared_ptr<Renderbuffer> colorRenderbuffer, depthRenderbuffer;

    Framebuffer();
    ~Framebuffer();

    void detachColor();
    void detachDepth();

    void attachColor(std::shared_ptr<Texture>, uint32_t = 0);
    void attachColor(std::shared_ptr<Renderbuffer>);

    void attachDepth(std::shared_ptr<Texture>, uint32_t = 0);
    void attachDepth(std::shared_ptr<Renderbuffer>);

    void setDrawBuffer(GLenum);
    void setReadBuffer(GLenum);
};

struct Model : public ResourceStorage::Object
{
    struct Material;
    struct Mesh;
    struct Animation;
    struct Node;

    std::shared_ptr<Node> rootNode;
    std::unordered_map<std::string, std::shared_ptr<Animation>> animations;
    std::vector<utils::Transform> boneTransforms;
    std::vector<std::string> boneNames;

    uint32_t numBones() const;
    bool calcBoneTransforms(const std::string&, float, std::vector<glm::mat3x4>&) const;
};

struct Model::Material
{
    std::pair<std::string, std::shared_ptr<Texture>> diffuseTexture = {"", nullptr};
    std::pair<std::string, std::shared_ptr<Texture>> normalTexture = {"", nullptr};
    std::pair<std::string, std::shared_ptr<Texture>> opacityTexture = {"", nullptr};
    std::pair<std::string, std::shared_ptr<Texture>> metallicOrSpecularTexture = {"", nullptr};
    std::pair<std::string, std::shared_ptr<Texture>> roughOrGlossTexture = {"", nullptr};
    bool isMetallicRoughWorkflow = true;
};

struct Model::Mesh
{
    std::shared_ptr<core::Mesh> mesh;
    std::shared_ptr<Material> material;

    Mesh(std::shared_ptr<core::Mesh> msh, std::shared_ptr<Material> mtl)
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

struct Model::Node : public utils::TreeNode<Node>
{
    utils::Transform transform;
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::int32_t boneIndex;

    Node(const utils::Transform& t = utils::Transform())
        : transform(t)
        , boneIndex(-1)
    {
    }
};

class Renderer
{
    NONCOPYBLE(Renderer)

public:
    Renderer(QOpenGLExtraFunctions&, GLuint);
    void initializeResources();

    static Renderer& instance();
    QOpenGLExtraFunctions& functions();
    GLuint defaultFbo() const;

    std::shared_ptr<RenderProgram> loadRenderProgram(const std::string&, const std::string&);
    std::shared_ptr<Texture> loadTexture(const std::string&);
    std::shared_ptr<Texture> createTexture2D(GLenum, GLint, GLint, GLenum, GLenum, const void*, const std::string& = "");
    std::shared_ptr<Texture> createTexture2DArray(GLenum, GLint, GLint, GLint, GLenum, GLenum, const void*, const std::string& = "");
    std::shared_ptr<Model> loadModel(const std::string&);
    std::shared_ptr<Model::Animation> loadAnimation(const std::string&);

    // binding
    void bindTexture(std::shared_ptr<Texture>, GLint);
    void bindUniformBuffer(std::shared_ptr<Buffer>, GLuint);

    void draw(std::shared_ptr<Drawable>, const utils::Transform&);
    void render(std::shared_ptr<Framebuffer>);

    void readPixel(std::shared_ptr<Framebuffer>, int, int, uint8_t&, uint8_t&, uint8_t&, uint8_t&, float&) const;

    void setViewport(const glm::ivec4&);
    void setViewMatrix(const glm::mat4x4&);
    void setProjectionMatrix(const glm::mat4x4&);
    void setClearColor(bool, const glm::vec4& = glm::vec4(0.f, 0.f, 0.f, 1.f));
    void setClearDepth(bool, float = 1.f);
    void setLightsBuffer(std::shared_ptr<Buffer>);
    void setShadowMaps(std::shared_ptr<Texture>);
    void setIBLMaps(std::shared_ptr<Texture>, std::shared_ptr<Texture>);

private:
    using DrawDataType = std::pair<std::shared_ptr<Drawable>, utils::Transform>;
    struct DrawDataComparator
    {
        using is_transparent = void;
        bool operator ()(const DrawDataType&, const DrawDataType&) const;
        bool operator ()(const DrawDataType&, LayerId) const;
    };
    using DrawDataContainer = std::multiset<DrawDataType, DrawDataComparator>;

    void renderShadowLayer(DrawDataContainer::iterator, DrawDataContainer::iterator);
    void renderSolidLayer(DrawDataContainer::iterator, DrawDataContainer::iterator);
    void renderTransparentLayer(DrawDataContainer::iterator, DrawDataContainer::iterator);
    void setupAndRender(DrawDataContainer::iterator, DrawDataContainer::iterator);

    GLbitfield calcClearMask() const;
    static std::string precompileShader(const QString& dir, QByteArray&);

    QOpenGLExtraFunctions& m_functions;
    GLuint m_defaultFbo;
    std::unique_ptr<ResourceStorage> m_resourceStorage;
    DrawDataContainer m_drawData;
    glm::mat4x4 m_projMatrix, m_viewMatrix, m_viewMatrixInverse, m_viewProjMatrix;
    glm::ivec4 m_viewport;
    glm::vec3 m_viewPosition;
    std::shared_ptr<Buffer> m_lightsBuffer = nullptr;
    std::shared_ptr<Texture> m_shadowMaps = nullptr;
    std::shared_ptr<Texture> m_IBLDiffuseMap = nullptr, m_IBLSpecularMap = nullptr, m_brdfLutMap;
    int32_t m_numIBLSpecularMapsMipmaps = 0;

    bool m_clearColorBit = true, m_clearDepthBit = true;
    glm::vec4 m_clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
    float m_clearDepth = 1.0f;

    friend class RenderWidget;
};

} // namespace
} // namespace

#endif // RENDERER_H
