#include <core/node.h>

#include "nodeprivate.h"
#include "sceneprivate.h"

namespace trash
{
namespace core
{

const utils::BoundingSphere NodePrivate::emptyLocalBoundingSphere = utils::BoundingSphere();

NodePrivate::NodePrivate(Node &node)
    : thisNode(node)
    , minimalBoundingSphere()
    , isGlobalTransformDirty(true)
    , isBoundingSphereDirty(true)
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

void NodePrivate::dirtyBoundingSphere()
{
    isBoundingSphereDirty = true;
    if (thisNode.parent())
        thisNode.parent()->m().dirtyBoundingSphere();
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

const utils::BoundingSphere &NodePrivate::getBoundingSphere()
{
    if (isBoundingSphereDirty)
    {
        boundingSphere = minimalBoundingSphere;
        boundingSphere += getLocalBoundingSphere();
        for (auto child : thisNode.children())
            boundingSphere += child->transform() * child->boundingSphere();
        isBoundingSphereDirty = false;
    }
    return boundingSphere;
}

const utils::Transform &NodePrivate::getGlobalTransform()
{
    if (isGlobalTransformDirty)
    {
        globalTransform = transform;

        auto parent = thisNode.parent();
        if (parent)
            globalTransform = parent->globalTransform() * globalTransform;

        isGlobalTransformDirty = false;
    }

    return globalTransform;
}

} // namespace
} // namespace
