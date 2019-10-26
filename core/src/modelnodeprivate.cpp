#include <core/node.h>

#include "modelnodeprivate.h"
#include "renderer.h"

class SkeletalMeshDrawable : public Drawable
{
public:
    SkeletalMeshDrawable(std::shared_ptr<RenderProgram> rp, std::shared_ptr<Model::Mesh> m, std::shared_ptr<UniformBuffer> mb);
    std::shared_ptr<RenderProgram> renderProgram() const override { return renderProgram_; }
    std::shared_ptr<UniformBuffer> bufferData() const override { return bufferData_; }
    std::shared_ptr<Texture> diffuseTexture() const override { return mesh->material->diffuseTexture; }
    GLuint vao() const override { return mesh->vao; }
    uint32_t numIndices() const override { return mesh->numIndices; }

    std::shared_ptr<Model::Mesh> mesh;
    std::shared_ptr<RenderProgram> renderProgram_;
    std::shared_ptr<UniformBuffer> bufferData_;
};

SkeletalMeshDrawable::SkeletalMeshDrawable(std::shared_ptr<RenderProgram> rp, std::shared_ptr<Model::Mesh> m, std::shared_ptr<UniformBuffer> mb)
    : mesh(m)
    , renderProgram_(rp)
    , bufferData_(mb)
{
}
#include <core/core.h>
#include <core/graphicscontroller.h>
#include "graphicscontrollerprivate.h"
ModelNodePrivate::ModelNodePrivate(Node &node, const std::string& modelName_)
    : NodePrivate(node)
    , modelName(modelName_)
    , timeOffset(0)
{
    auto& renderer = Core::instance().graphicsController().m().renderer;

    model = renderer.loadModel(modelName);

    if (model->numBones())
        renderProgram = renderer.loadRenderProgram(":/res/shader_anim.vert", ":/res/shader.frag");
    else
        renderProgram = renderer.loadRenderProgram(":/res/shader.vert", ":/res/shader.frag");

    modelBuffer = renderer.createUniformBuffer(model->numBones()*48, GL_DYNAMIC_DRAW);
}

void ModelNodePrivate::doUpdate(Renderer& renderer, uint64_t time, uint64_t)
{
    std::vector<Transform> bones;
    model->calcBoneTransforms(animationName, (time + timeOffset) * 0.001f, bones);

    void *pData = renderer.mapUniformBuffer(modelBuffer, 0, static_cast<GLsizeiptr>(bones.size()*48), GL_MAP_WRITE_BIT);
    for (size_t i = 0; i < bones.size(); ++i)
        reinterpret_cast<glm::mat3x4*>(pData)[i] = glm::transpose(bones[i].operator glm::mat4x4());
    renderer.unmapUniformBuffer();

    std::function<void(std::shared_ptr<Model::Node>, const Transform&)> renderModel = [this, &renderer, &renderModel](std::shared_ptr<Model::Node> node, const Transform& parentTransform)
    {
        Transform transform = parentTransform * node->transform;

        for (auto mesh : node->meshes)
            renderer.draw(0, std::make_shared<SkeletalMeshDrawable>(renderProgram, mesh, modelBuffer), thisNode.globalTransform() * transform);

        for (auto child : node->children())
            renderModel(child, transform);
    };
    renderModel(model->rootNode, Transform());
}
