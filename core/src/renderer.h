#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
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
class Drawable;
class MeshDrawable;
class SphereDrawable;
class FrustumDrawable;
struct BoundingSphere;
struct Frustum;

const int32_t LayerId_ComputeAnimations = -1;


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
    GLsizei numVertices;
    GLsizei numComponents;
    std::unordered_map<VertexAttribute, GLsizei> declaration;

    VertexBuffer(GLsizei, GLsizei, float*, GLenum);

    void declareAttribute(VertexAttribute, GLsizei);
};

struct IndexBuffer : public Buffer
{
    GLsizei numIndices;
    GLenum primitiveType;

    IndexBuffer(GLenum, GLsizei, uint32_t*, GLenum);
};

struct Mesh
{
    NONCOPYBLE(Mesh)

    GLuint id;
    std::unordered_set<std::shared_ptr<VertexBuffer>> vertexBuffers;
    std::unordered_set<std::shared_ptr<IndexBuffer>> indexBuffers;

    Mesh();
    ~Mesh();

    void attachVertexBuffer(std::shared_ptr<VertexBuffer>);
    void attachVertexBuffer(std::shared_ptr<VertexBuffer>, const std::unordered_set<VertexAttribute>&); // attach only a few attributes from vertexbuffer

    void attachIndexBuffer(std::shared_ptr<IndexBuffer>);

    void tmp(std::shared_ptr<VertexBuffer>);
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
    BoundingSphere calcBoundingSphere() const;
};

struct Model::Material
{
    std::shared_ptr<Texture> diffuseTexture;
};

struct Model::Mesh
{
    std::shared_ptr<::Mesh> mesh;
    std::shared_ptr<Material> material;
    BoundingSphere boundingSphere;
    bool hasAnimatedAttributes;

    Mesh(std::shared_ptr<::Mesh> msh, std::shared_ptr<Material> mtl, const BoundingSphere& bs, bool at)
        : mesh(msh)
        , material(mtl)
        , boundingSphere(bs)
        , hasAnimatedAttributes(at)
    {}
};

struct Model::Animation
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

    Node(const Transform& t)
        : transform(t)
        , boneIndex(-1)
    {
    }
};

class Layer
{
    NONCOPYBLE(Layer)
public:
    Layer() {}

    bool m_transformFeedback = false;
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

    std::shared_ptr<Layer> getOrCreateLayer(int32_t);

    // transform feedback
    void beginTransformFeedback(GLenum);
    void endTransformFeedback();

    // binding
    void bindTexture(std::shared_ptr<Texture>, GLint);
    void bindUniformBuffer(std::shared_ptr<Buffer>, GLuint);
    void bindTransformFeedbackBuffer(std::shared_ptr<Buffer>, GLuint);

    // rendering
    std::shared_ptr<MeshDrawable> createMeshDrawable(std::shared_ptr<RenderProgram>, std::shared_ptr<Model::Mesh>, std::shared_ptr<VertexBuffer>) const;
    std::shared_ptr<SphereDrawable> createSphereDrawable(uint32_t, const BoundingSphere&, const glm::vec4&) const;
    std::shared_ptr<FrustumDrawable> createFrustumDrawable(const Frustum&, const glm::vec4&) const;

    void draw(int32_t, std::shared_ptr<Drawable>, const Transform&);

    void setViewMatrix(const glm::mat4x4&);
    void setProjectionMatrix(float, float, float);

    const glm::mat4x4& projectionMatrix() const;
    const glm::ivec4 viewport() const;

    void drawMesh(std::shared_ptr<Mesh>, std::shared_ptr<RenderProgram>);

private:
    using DrawDataType = std::multimap<int32_t, std::pair<std::shared_ptr<Drawable>, Transform>>;

    void resize(int, int);
    void render();
    void renderLayer(DrawDataType::iterator, DrawDataType::iterator);

    QOpenGLExtraFunctions& m_functions;
    DrawDataType m_drawData;
    std::unordered_map<int32_t, std::shared_ptr<Layer>> m_layers;
    glm::mat4x4 m_projMatrix;
    glm::mat4x4 m_viewMatrix;
    glm::ivec4 m_viewport;
    float m_fov = glm::pi<float>() * 0.25f, m_zNear = 0.5f, m_zFar = 10000.0f;


    std::shared_ptr<Layer> m_standardLayer;
    std::shared_ptr<Texture> m_standardTexture;
    std::shared_ptr<RenderProgram> m_coloredRenderProgram;
    std::shared_ptr<RenderProgram> m_skeletalAnimationProgram;

    friend class RenderWidget;
};

#endif // RENDERER_H
