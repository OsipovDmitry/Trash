#include <core/node.h>

#include "nodeprivate.h"
#include "sceneprivate.h"

#include "renderer.h"
#include "drawables.h"

namespace trash
{
namespace core
{

const utils::BoundingBox NodePrivate::emptyLocalBoundingBox = utils::BoundingBox();

NodePrivate::NodePrivate(Node &node)
    : thisNode(node)
    , minimalBoundingBox()
    , isGlobalTransformDirty(true)
    , isBoundingBoxDirty(true)
{
}

NodePrivate::~NodePrivate()
{
}

void NodePrivate::dirtyGlobalTransform()
{
    isGlobalTransformDirty = true;
    for (auto child : thisNode.children())
        child->m().dirtyGlobalTransform();
}

void NodePrivate::dirtyBoundingBox()
{
    isBoundingBoxDirty = true;
    if (thisNode.parent())
        thisNode.parent()->m().dirtyBoundingBox();
}

void NodePrivate::dirtyLightIndices()
{
    doDirtyLightIndices();
    for (auto child : thisNode.children())
        child->m().dirtyLightIndices();
}

void NodePrivate::dirtyShadowMaps()
{
    doDirtyShadowMaps();
    for (auto child : thisNode.children())
        child->m().dirtyShadowMaps();
}

void NodePrivate::doUpdate(uint64_t, uint64_t)
{
    auto& renderer = Renderer::instance();
    renderer.draw(std::make_shared<BoxDrawable>(getBoundingBox(), glm::vec4(.0f, .8f, .0f, 1.0f)), getGlobalTransform());
}

Scene *NodePrivate::getScene() const
{
    Node *node = &thisNode;

    while (node->parent())
        node = node->parent();

    if (auto sceneRootNode = dynamic_cast<SceneRootNode*>(node))
    {
        return sceneRootNode->scene();
    }

    return nullptr;
}

const utils::BoundingBox &NodePrivate::getBoundingBox()
{
    if (isBoundingBoxDirty)
    {
        boundingBox = minimalBoundingBox;
        boundingBox += getLocalBoundingBox();
        for (auto child : thisNode.children())
            boundingBox += child->transform() * child->boundingBox();
        isBoundingBoxDirty = false;
    }
    return boundingBox;
}

const utils::Transform &NodePrivate::getGlobalTransform()
{
    if (isGlobalTransformDirty)
    {
        auto parent = thisNode.parent();
        globalTransform = parent ? parent->globalTransform() * transform : transform;

        isGlobalTransformDirty = false;
    }

    return globalTransform;
}

} // namespace
} // namespace
