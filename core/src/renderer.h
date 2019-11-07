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
struct BoundingSphere;
struct Frustum;

struct RenderProgram : public ResourceStorage::Object
{
    GLuint id;
    RenderProgram(GLuint id_) : id(id_) {}
    ~RenderProgram() override;

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

struct GLBuffer
{
    GLenum target;
    GLuint id;

    GLBuffer(GLenum, GLsizeiptr, GLvoid*, GLenum);
    virtual ~GLBuffer();

    void *map(GLintptr, GLsizeiptr, GLbitfield);
    void unmap();
};

struct VertexBuffer : public GLBuffer
{
    VertexBuffer(GLsizeiptr, GLvoid*, GLenum);
};

struct IndexBuffer : public GLBuffer
{
    GLsizei numIndices;
    GLenum primitiveType;

    IndexBuffer(GLenum, GLsizei, uint32_t*, GLenum);
};

struct UniformBuffer : public GLBuffer
{
    UniformBuffer(GLsizeiptr, GLvoid*, GLenum);
};

struct Mesh
{
    GLuint id;
    std::unordered_map<VertexAttribute, std::tuple<std::shared_ptr<VertexBuffer>, GLint, GLsizei, GLsizei>> attributes;
    std::unordered_set<std::shared_ptr<IndexBuffer>> indexBuffers;

    Mesh();
    ~Mesh();

    void attachVertexBuffer(VertexAttribute, std::shared_ptr<VertexBuffer>, GLint, GLsizei, GLsizei);
    void attachIndexBuffer(std::shared_ptr<IndexBuffer>);
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

    Mesh(std::shared_ptr<::Mesh> msh, std::shared_ptr<Material> mtl, const BoundingSphere& bs)
        : mesh(msh)
        , material(mtl)
        , boundingSphere(bs)
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

    // binding
    void bindTexture(std::shared_ptr<Texture>, GLint);
    void bindUniformBuffer(std::shared_ptr<UniformBuffer>, GLuint);

    // rendering
    std::shared_ptr<Drawable> createSkeletalMeshDrawable(std::shared_ptr<RenderProgram>, std::shared_ptr<Model::Mesh>, std::shared_ptr<UniformBuffer>) const;
    std::shared_ptr<Drawable> createSphereDrawable(uint32_t, const BoundingSphere&, const glm::vec4&) const;
    std::shared_ptr<Drawable> createFrustumDrawable(const Frustum&, const glm::vec4&) const;

    void draw(uint32_t, std::shared_ptr<Drawable>, const Transform&);

    void setViewMatrix(const glm::mat4x4&);
    void setProjectionMatrix(float, float, float);

    const glm::mat4x4& projectionMatrix() const;

private:
    using DrawDataType = std::multimap<uint32_t, std::pair<std::shared_ptr<Drawable>, Transform>>;

    void resize(int, int);
    void render();
    void renderLayer(DrawDataType::iterator, DrawDataType::iterator);

    QOpenGLExtraFunctions& m_functions;
    glm::mat4x4 m_projMatrix;
    glm::mat4x4 m_viewMatrix;
    float m_fov = glm::pi<float>() * 0.25f, m_zNear = 0.5f, m_zFar = 10000.0f;
    int32_t m_windowWidth, m_windowHeight;
    DrawDataType m_drawData;

    std::shared_ptr<Texture> m_standardTexture;
    std::shared_ptr<RenderProgram> m_coloredRenderProgram;

    friend class RenderWidget;
};

#endif // RENDERER_H
