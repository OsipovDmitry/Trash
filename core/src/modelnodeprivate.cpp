#include <core/core.h>
#include <core/node.h>

#include "coreprivate.h"
#include "modelnodeprivate.h"
#include "renderwidget.h"
#include "renderer.h"

ModelNodePrivate::ModelNodePrivate(Node &node, const std::string& modelName_)
    : NodePrivate(node)
    , modelName(modelName_)
    , timeOffset(0)
{
    auto& renderer = Renderer::instance();

    model = renderer.loadModel(modelName);

    if (model->numBones())
        renderProgram = renderer.loadRenderProgram(":/res/shader_anim.vert", ":/res/shader.frag");
    else
        renderProgram = renderer.loadRenderProgram(":/res/shader.vert", ":/res/shader.frag");

    modelBuffer = std::make_shared<UniformBuffer>(model->numBones()*48, nullptr, GL_DYNAMIC_DRAW);
}

void ModelNodePrivate::doUpdate(uint64_t time, uint64_t dt)
{
    NodePrivate::doUpdate(time, dt);

    auto& renderer = Renderer::instance();

    std::vector<Transform> bones;
    model->calcBoneTransforms(animationName, (time + timeOffset) * 0.001f, bones);

    void *pData = modelBuffer->map(0, static_cast<GLsizeiptr>(bones.size()*48), GL_MAP_WRITE_BIT);
    for (size_t i = 0; i < bones.size(); ++i)
        reinterpret_cast<glm::mat3x4*>(pData)[i] = glm::transpose(bones[i].operator glm::mat4x4());
    modelBuffer->unmap();

    std::function<void(std::shared_ptr<Model::Node>, const Transform&)> renderModel = [this, &renderer, &renderModel](std::shared_ptr<Model::Node> node, const Transform& parentTransform)
    {
        Transform transform = parentTransform * node->transform;

        for (auto mesh : node->meshes) {
            renderer.draw(0, renderer.createSkeletalMeshDrawable(renderProgram, mesh, modelBuffer), thisNode.globalTransform() * transform);
            //renderer.draw(0, renderer.createSphereDrawable(5, mesh->boundingSphere, glm::vec4(.2f, .2f, .2f, 1.0f)), thisNode.globalTransform() * transform);
        }

        for (auto child : node->children())
            renderModel(child, transform);
    };
    renderModel(model->rootNode, Transform());
}

BoundingSphere ModelNodePrivate::calcLocalBoundingSphere()
{
    return model->calcBoundingSphere();
}
