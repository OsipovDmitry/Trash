#include <queue>

#include <core/modelnode.h>

#include "modelnodeprivate.h"
#include "renderer.h"
#include "drawables.h"

ModelNode::ModelNode(const std::string &filename)
    : Node(new ModelNodePrivate(*this))
{
    auto& mPrivate = m();
    auto& renderer = Renderer::instance();

    mPrivate.model = renderer.loadModel(filename);

    if (mPrivate.model->numBones())
        mPrivate.bonesBuffer = std::make_shared<Buffer>(mPrivate.model->numBones()*sizeof(glm::mat3x4), nullptr, GL_DYNAMIC_DRAW);

    BoundingSphere minimalBoundingSphere;
    std::queue<std::pair<std::shared_ptr<Model::Node>, Transform>> nodes;
    nodes.push(std::make_pair(mPrivate.model->rootNode, Transform()));

    while (!nodes.empty())
    {
        auto& nodeData = nodes.front();
        std::shared_ptr<Model::Node> node = nodeData.first;
        Transform transform = nodeData.second * node->transform;
        nodes.pop();

        for (auto mesh : node->meshes)
        {
            std::shared_ptr<Texture> diffuseTexture, normalTexture;

            if (mesh->material)
            {
                diffuseTexture = mesh->material->diffuseTexture.second;
                normalTexture = mesh->material->normalTexture.second;
            }

            auto meshNode = std::make_shared<Node>();
            meshNode->setTransform(transform);
            meshNode->m().addDrawable(std::make_shared<TexturedMeshDrawable>(mesh->mesh, diffuseTexture, normalTexture, mPrivate.bonesBuffer));
            attach(meshNode);

            minimalBoundingSphere += transform * mesh->mesh->boundingSphere;
        }

        for (auto child : node->children())
            nodes.push(std::make_pair(child, transform));
    }

    for (auto meshNode : children())
        meshNode->m().minimalBoundingSphere = meshNode->transform().inverse() * minimalBoundingSphere;
}

void ModelNode::showBones(bool state)
{
    m().showBones = state;
}

//void ModelNode::copyAnimation(std::shared_ptr<ModelNode> anotherModelNode, const std::string& animName)
//{
//    auto anotherModel = anotherModelNode->m().model;
//    if (!anotherModel)
//        return;

//    auto animIter = anotherModel->animations.find(animName);
//    if (animIter == anotherModel->animations.end())
//        return;

//    auto model = m().model;
//    if (!model)
//        return;

//    model->animations[animIter->first] = animIter->second;
//}

void ModelNode::playAnimation(const std::string& animationName, uint64_t timeOffset)
{
    auto& privateData = m();

    if (!privateData.model->animations.count(animationName))
    {
        auto anim = Renderer::instance().loadAnimation(animationName + ".anim");
        assert(anim != nullptr);
        privateData.model->animations.insert({animationName, anim});
    }

    privateData.animationName = animationName;
    privateData.timeOffset = timeOffset;
    privateData.startAnimation = true;
}

uint64_t ModelNode::animationTime(const std::string& animationName) const
{
    auto anim = m().model->animations[animationName];
    if (!anim)
        return 0;
    return static_cast<uint64_t>(anim->duration / anim->framesPerSecond * 1000.0f);
}
