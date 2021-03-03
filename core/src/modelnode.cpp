#include <queue>

#include <core/modelnode.h>
#include <core/drawablenode.h>

#include "modelnodeprivate.h"
#include "drawablenodeprivate.h"
#include "sceneprivate.h"
#include "renderer.h"
#include "drawables.h"

namespace trash
{
namespace core
{

ModelNode::ModelNode(const std::string &filename)
    : Node(new ModelNodePrivate(*this))
{
    auto& mPrivate = m();
    auto& renderer = Renderer::instance();

    mPrivate.model = renderer.loadModel(filename);

    if (mPrivate.model->numBones())
        mPrivate.bonesBuffer = std::make_shared<Buffer>(mPrivate.model->numBones()*sizeof(glm::mat3x4), nullptr, GL_DYNAMIC_DRAW);

    utils::BoundingBox minimalBoundingBox;
    std::queue<std::pair<std::shared_ptr<Model::Node>, utils::Transform>> nodes;
    nodes.push(std::make_pair(mPrivate.model->rootNode, utils::Transform()));

    while (!nodes.empty())
    {
        auto& nodeData = nodes.front();
        std::shared_ptr<Model::Node> node = nodeData.first;
        utils::Transform transform = nodeData.second * node->transform;
        nodes.pop();

        for (auto mesh : node->meshes)
        {
            std::shared_ptr<Texture> diffuseTexture, opacityTexture, normalTexture, metallicTexture, roughTexture;

            if (mesh->material)
            {
                diffuseTexture = mesh->material->baseColorMap.second;
                opacityTexture = mesh->material->opacityMap.second;
                normalTexture = mesh->material->normalMap.second;
                metallicTexture = mesh->material->metallicMap.second;
                roughTexture = mesh->material->roughnessMap.second;
            }

            auto meshNode = std::make_shared<DrawableNode>();
            auto& meshNodePrivate = meshNode->m();
            meshNode->setTransform(transform);
            meshNodePrivate.addDrawable(std::make_shared<StandardDrawable>(mesh->mesh,
                                                                         mPrivate.bonesBuffer,
                                                                         glm::vec4(1.f, 1.f, 1.f, 1.f),
                                                                         glm::vec2(1.f, 1.f),
                                                                         diffuseTexture,
                                                                         opacityTexture,
                                                                         normalTexture,
                                                                         metallicTexture,
                                                                         roughTexture,
                                                                         meshNodePrivate.getLightIndices()));
            attach(meshNode);

            minimalBoundingBox += transform * mesh->mesh->boundingBox;
        }

        for (auto child : node->children())
            nodes.push(std::make_pair(child, transform));
    }

    for (auto meshNode : children())
        meshNode->m().minimalBoundingBox = meshNode->transform().inverted() * minimalBoundingBox;
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

void ModelNode::setAnimationFrame(const std::string& animationName, uint64_t animationTime)
{
    auto& privateData = m();

    if (!privateData.model->animations.count(animationName))
    {
        auto anim = Renderer::instance().loadAnimation(animationName + ".anim");
        assert(anim != nullptr);
        privateData.model->animations.insert({animationName, anim});
    }

    if (privateData.animationName == animationName && privateData.animationTime == animationTime)
        return;

    privateData.animationName = animationName;
    privateData.animationTime = animationTime;

    ScenePrivate::dirtyNodeShadowMaps(*this);

    std::vector<glm::mat3x4> bones;
    privateData.model->calcBoneTransforms(animationName, animationTime * 0.001f, bones);
    privateData.bonesBuffer->setSubData(0, static_cast<GLsizeiptr>(bones.size()*sizeof(glm::mat3x4)), bones.data());
}

uint64_t ModelNode::animationTime(const std::string& animationName) const
{
    auto& privateData = m();

    if (!privateData.model->animations.count(animationName))
    {
        auto anim = Renderer::instance().loadAnimation(animationName + ".anim");
        assert(anim != nullptr);
        privateData.model->animations.insert({animationName, anim});
    }

    auto anim = privateData.model->animations[animationName];
    if (!anim)
        return 0;
    return static_cast<uint64_t>(anim->duration / anim->framesPerSecond * 1000.0f + .5f);
}

} // namespace
} // namespace
