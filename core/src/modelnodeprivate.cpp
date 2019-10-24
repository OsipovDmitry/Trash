#include <core/node.h>

#include "modelnodeprivate.h"
#include "renderer.h"

class SkeletalMeshDrawable : public Drawable
{
public:
    SkeletalMeshDrawable(std::shared_ptr<RenderProgram> rp, std::shared_ptr<Model::Mesh> m);
    std::shared_ptr<RenderProgram> renderProgram() const override { return renderProgram_; }
    std::shared_ptr<Texture> diffuseTexture() const override { return mesh->material->diffuseTexture; }
    GLuint vao() const override { return mesh->vao; }
    uint32_t numIndices() const override { return mesh->numIndices; }

    std::shared_ptr<Model::Mesh> mesh;
    std::shared_ptr<RenderProgram> renderProgram_;
};

SkeletalMeshDrawable::SkeletalMeshDrawable(std::shared_ptr<RenderProgram> rp, std::shared_ptr<Model::Mesh> m)
    : mesh(m)
    , renderProgram_(rp)
{
}

ModelNodePrivate::ModelNodePrivate(Node &node, const std::string& modelName_)
    : NodePrivate(node)
    , modelName(modelName_)
{
}

void ModelNodePrivate::doUpdate(Renderer& renderer, uint64_t time, uint64_t)
{
    if (!model)
        model = renderer.loadModel(modelName);

    if (!renderProgram)
        renderProgram = renderer.loadRenderProgram(":/res/shader.vert", ":/res/shader.frag");


    std::function<void(std::shared_ptr<Model::Node>, const Transform&)> renderModel = [this, &renderer, &renderModel](std::shared_ptr<Model::Node> node, const Transform& parentTransform)
    {
        Transform transform = parentTransform * node->transform;

        for (auto mesh : node->meshes)
            renderer.draw(0, std::make_shared<SkeletalMeshDrawable>(renderProgram, mesh), thisNode.globalTransform() * transform);

        for (auto child : node->children())
            renderModel(child, transform);
    };
    renderModel(model->rootNode, Transform());
}
