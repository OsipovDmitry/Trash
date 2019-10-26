#include <core/modelnode.h>

#include "modelnodeprivate.h"
#include "renderer.h"

ModelNode::ModelNode(const std::string &filename)
    : Node(new ModelNodePrivate(*this, filename))
{
}

void ModelNode::showBones(bool state)
{
    m().showBones = state;
}

void ModelNode::copyAnimation(std::shared_ptr<ModelNode> anotherModelNode, const std::string& animName)
{
    auto anotherModel = anotherModelNode->m().model;
    if (!anotherModel)
        return;

    auto animIter = anotherModel->animations.find(animName);
    if (animIter == anotherModel->animations.end())
        return;

    auto model = m().model;
    if (!model)
        return;

    model->animations[animIter->first] = animIter->second;
}

void ModelNode::playAnimation(const std::string& animationName, uint32_t timeOffset)
{
    auto& privateData = m();
    privateData.animationName = animationName;
    privateData.timeOffset = timeOffset;
}
