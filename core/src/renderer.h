#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <deque>
#include <functional>

#include <QtOpenGL/QGL>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <core/types.h>
#include <core/forwarddecl.h>
#include <utils/tree.h>
#include <utils/transform.h>
#include <utils/boundingbox.h>
#include <utils/enumclass.h>
#include <utils/noncopyble.h>

#include "resourcestorage.h"
#include "typesprivate.h"


class QOpenGLExtraFunctions;

namespace trash
{
namespace core
{

class Drawable;
class BlurDrawable;
class CombineDrawable;

class AbstractUniform
{
public:
    virtual ~AbstractUniform();
};

template <typename T>
class Uniform : public AbstractUniform
{
public:
    Uniform(const T& value) : m_data(value) {}
    ~Uniform() = default;

    const T& get() const { return m_data; }
    void set(const T& value) { m_data = value; }

private:
    T m_data;
};

struct RenderProgram : public ResourceStorage::Object
{
    GLuint id;
    std::unordered_map<UniformId, GLint> uniforms;

    RenderProgram(GLuint id_);
    ~RenderProgram() override;

    void setupTransformFeedback(const std::vector<std::string>&, GLenum);

    GLint uniformBufferDataSize(GLuint);
    std::unordered_map<std::string, GLint> uniformBufferOffsets(GLuint); // it works but don't use it better

    static UniformId uniformIdByName(const std::string&);
};

struct Texture : public ResourceStorage::Object
{
    GLenum target;
    GLuint id;

    Texture(GLuint id_, GLenum target_) : target(target_), id(id_) {}
    ~Texture() override;

    void setFilter(int32_t); // 1 - nearest // 2 - linear // 3 - trilinear
    void setWrap(GLenum);
    void setCompareMode(GLenum);
    void setCompareFunc(GLenum);
    void setBorderColor(const glm::vec4&);

    uint32_t maxMipmapLevel() const;
    void setMaxMipmapLevel(uint32_t);
    void generateMipmaps();

    static bool stringToInternalFormat(const std::string& str, GLenum& internalFormat);
    static bool formatAndTypeToInternalFormat(GLenum format, GLenum type, GLenum& internalFormat);
    static bool stringToWrap(const std::string& str, GLenum& wrap);
};

struct Buffer
{
    NONCOPYBLE(Buffer)

    GLuint id;

    Buffer(GLsizeiptr, const GLvoid*, GLenum);
    virtual ~Buffer();

    int64_t size() const;

    void setSubData(GLintptr, GLsizeiptr, const void*);

    void *map(GLintptr, GLsizeiptr, GLbitfield);
    static void unmap();

    const void *cpuData() const;
    void clearCpuData();

protected:
    void *m_cpuData;
    bool m_cpuDataIsDirty;

};

struct VertexBuffer : public Buffer
{
    uint32_t numVertices;
    uint32_t numComponents;

    VertexBuffer(uint32_t, uint32_t, const float*, GLenum);
};

struct IndexBuffer : public Buffer
{
    uint32_t numIndices;
    GLenum primitiveType;

    IndexBuffer(GLenum, uint32_t, const uint32_t*, GLenum);
};

struct Mesh
{
    NONCOPYBLE(Mesh)

    GLuint id;
    std::unordered_map<VertexAttribute, std::shared_ptr<VertexBuffer>> attributesDeclaration;
    std::unordered_set<std::shared_ptr<IndexBuffer>> indexBuffers;
    utils::BoundingBox boundingBox;
    uint32_t numInstances;

    Mesh();
    ~Mesh();

    void declareVertexAttribute(VertexAttribute, std::shared_ptr<VertexBuffer>, uint32_t = 0);
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
    struct RenderTarget {
        RenderTarget(std::shared_ptr<Texture> t) : texture(t) {}
        RenderTarget(std::shared_ptr<Renderbuffer> rb) : renderbuffer(rb) {}

        bool isTexture() const { return texture != nullptr; }
        bool isRenderbuffer() const { return renderbuffer != nullptr; }

        std::shared_ptr<Texture> texture = nullptr;
        std::shared_ptr<Renderbuffer> renderbuffer = nullptr;
    };

    NONCOPYBLE(Framebuffer)

    GLuint id;
    std::array<std::shared_ptr<RenderTarget>, 8> colorAttachments;
    std::shared_ptr<RenderTarget> depthAttachment;
    std::shared_ptr<RenderTarget> depthStencilAttachment;

    Framebuffer();
    ~Framebuffer();

    void detachColor(size_t);
    void detachDepth();
    void detachDepthStencil();

    void attachColor(size_t, std::shared_ptr<Texture>, uint32_t, uint32_t);
    void attachColor(size_t, std::shared_ptr<Renderbuffer>);

    void attachDepth(std::shared_ptr<Texture>, uint32_t, uint32_t);
    void attachDepth(std::shared_ptr<Renderbuffer>);

    void attachDepthStencil(std::shared_ptr<Texture>, uint32_t, uint32_t);
    void attachDepthStencil(std::shared_ptr<Renderbuffer>);

    void drawBuffers(const std::vector<GLenum>&);

    bool isComplete() const;
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
    std::pair<std::string, std::shared_ptr<Texture>> baseColorMap = {"", nullptr};
    std::pair<std::string, std::shared_ptr<Texture>> normalMap = {"", nullptr};
    std::pair<std::string, std::shared_ptr<Texture>> opacityMap = {"", nullptr};
    std::pair<std::string, std::shared_ptr<Texture>> metallicMap = {"", nullptr};
    std::pair<std::string, std::shared_ptr<Texture>> roughnessMap = {"", nullptr};
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

struct Font : public ResourceStorage::Object
{
    struct Character { uint16_t x, y, width, height, advance; int16_t originX, originY;
                       Character(uint16_t x_, uint16_t y_, uint16_t w_, uint16_t h_, int16_t ox_, int16_t oy_, uint16_t a_) :
                           x(x_), y(y_), width(w_), height(h_), originX(ox_), originY(oy_), advance(a_) {}};

    std::shared_ptr<Texture> texture;
    std::string name = "";
    std::unordered_map<char, std::shared_ptr<Character>> characters;
    uint16_t size = 0;
    uint16_t width = 0, height = 0;
    bool isBold = false;
    bool isItalic = false;
};

class RenderInfo
{
public:
    RenderInfo(const glm::mat4x4&, const glm::mat4x4&);

    const glm::mat4x4& viewMatrix() const { return m_viewMatrix; }
    const glm::mat4x4& viewMatrixInverse() const { return m_viewMatrixInverse; }
    const glm::mat4x4& projMatrix() const { return m_projMatrix; }
    const glm::mat4x4& projMatrixInverse() const { return m_projMatrixInverse; }
    const glm::mat4x4& viewProjMatrix() const { return m_viewProjMatrix; }
    const glm::mat4x4& viewProjMatrixInverse() const { return m_viewProjMatrixInverse; }
    const glm::vec3& viewPosition() const { return m_viewPosition; }
    const glm::vec3& viewXDirection() const { return m_viewXDirection; }
    const glm::vec3& viewYDirection() const { return m_viewYDirection; }
    const glm::vec3& viewZDirection() const { return m_viewZDirection; }

    void setLightsBuffer(std::shared_ptr<Buffer> buffer) { m_lightsBuffer = buffer; }
    std::shared_ptr<Buffer> lightsBuffer() const { return m_lightsBuffer; }

    void setShadowMaps(std::shared_ptr<Texture> maps) { m_shadowMaps = maps; }
    std::shared_ptr<Texture> shadowMaps() const { return m_shadowMaps; }

    void setIBLData(std::shared_ptr<Texture> diffuse, std::shared_ptr<Texture> specular, std::shared_ptr<Texture> brdf, float contribution) {
        m_IBLDiffuseMap = diffuse;
        m_IBLSpecularMap = specular;
        m_brdfLutMap = brdf;
        m_IBLContribution = contribution;
        m_maxIBLSpecularMapMipmapLevel = m_IBLSpecularMap ? m_IBLSpecularMap->maxMipmapLevel() : 0;
    }
    std::shared_ptr<Texture> IBLDiffuseMap() const { return m_IBLDiffuseMap; }
    std::shared_ptr<Texture> IBLSpecularMap() const { return m_IBLSpecularMap; }
    std::shared_ptr<Texture> brdfLutMap() const { return m_brdfLutMap; }
    int32_t maxIBLSpecularMapMipmapLevel() const { return m_maxIBLSpecularMapMipmapLevel; }
    float IBLContribution() const { return m_IBLContribution; }

private:
    glm::mat4x4 m_viewMatrix;
    glm::mat4x4 m_viewMatrixInverse;
    glm::mat4x4 m_projMatrix;
    glm::mat4x4 m_projMatrixInverse;
    glm::mat4x4 m_viewProjMatrix;
    glm::mat4x4 m_viewProjMatrixInverse;
    glm::vec3 m_viewPosition;
    glm::vec3 m_viewXDirection, m_viewYDirection, m_viewZDirection;

    std::shared_ptr<Buffer> m_lightsBuffer;
    std::shared_ptr<Texture> m_shadowMaps;
    std::shared_ptr<Texture> m_IBLDiffuseMap, m_IBLSpecularMap, m_brdfLutMap;
    int32_t m_maxIBLSpecularMapMipmapLevel;
    float m_IBLContribution;
};

using RenderSurface = std::pair<std::shared_ptr<Framebuffer>, glm::uvec2>;

class Renderer
{
    NONCOPYBLE(Renderer)

public:
    Renderer(QOpenGLExtraFunctions&, GLuint);
    void initialize();
    void resize(int, int);

    const glm::uvec2& viewportSize() const;

    static Renderer& instance();
    QOpenGLExtraFunctions& functions();
    GLuint defaultFbo() const;

    std::shared_ptr<RenderProgram> loadRenderProgram(const std::string&, const std::string&, const std::map<std::string, std::string>& = std::map<std::string, std::string>());
    std::shared_ptr<Texture> loadTexture(const std::string&);
    std::shared_ptr<Texture> createTexture2D(GLenum, GLint, GLint, GLenum, GLenum, const void*, uint32_t, const std::string& = "");
    std::shared_ptr<Texture> createTexture2DArray(GLenum, GLint, GLint, GLint, GLenum, GLenum, const void*, const std::string& = "");
    std::shared_ptr<Model> loadModel(const std::string&);
    std::shared_ptr<Model::Animation> loadAnimation(const std::string&);
    std::shared_ptr<Font> loadFont(const std::string&);

    // binding
    void bindTexture(std::shared_ptr<Texture>, GLint);
    void bindUniformBuffer(std::shared_ptr<Buffer>, GLuint);

    void draw(std::shared_ptr<Drawable>, const utils::Transform&, uint32_t);
    void clear();

    void renderForward(const RenderInfo&);
    void renderDeffered(const RenderInfo&);

    void renderShadows(const RenderInfo&, std::shared_ptr<Framebuffer>, const glm::uvec2&);
    void renderIds(const RenderInfo&, std::shared_ptr<Framebuffer>, const glm::uvec2&);

    void readPixel(std::shared_ptr<Framebuffer>, GLenum, int, int, GLenum, GLenum, GLvoid*) const;

private:
    using DrawDataType = std::tuple<std::shared_ptr<Drawable>, utils::Transform, uint32_t>;
    using DrawDataLayerContainer = std::deque<DrawDataType>;
    using DrawDataContainer = std::array<DrawDataLayerContainer, numElementsLayerId()>;

    void setupViewportSize(const glm::uvec2&);
    void setupUniforms(const DrawDataType&, DrawableRenderProgramId, const RenderInfo&);
    void renderMesh(std::shared_ptr<Mesh>);
    void resizeRenderSurfaces(const glm::uvec2&);

    static std::string precompileShader(const QString& dir, QByteArray&, const std::map<std::string, std::string>&);

    QOpenGLExtraFunctions& m_functions;
    GLuint m_defaultFbo;
    std::unique_ptr<ResourceStorage> m_resourceStorage;
    DrawDataContainer m_drawData;
    glm::uvec2 m_cachedViewportSize, m_currentViewportSize;

    RenderSurface m_hdrRenderSurface;
    RenderSurface m_gRenderSurface;
    RenderSurface m_ssaoRenderSurface, m_bloomRenderSurface;
    std::vector<RenderSurface> m_ssaoBlurRenderSurface, m_bloomBlurRenderSurface;
    std::vector<RenderSurface> m_ssaoCombineRenderSurface, m_bloomCombineRenderSurface;

    std::shared_ptr<Mesh> m_fullscreenQuad;
    std::shared_ptr<Drawable> m_backgroundDrawable, m_ssaoDrawable, m_bloomDrawable, m_postEffectDrawable;
    std::shared_ptr<BlurDrawable> m_ssaoBlurDrawable, m_bloomBlurDrawable;
    std::shared_ptr<CombineDrawable> m_ssaoCombineDrawable, m_bloomCombineDrawable;
    const uint32_t m_ssaoBlurNumPasses, m_bloomBlurNumPasses;
    const float m_ssaoContribution;
    const bool m_isBloomEnabled;

    friend class RenderWidget;
};

} // namespace
} // namespace


#endif // RENDERER_H
