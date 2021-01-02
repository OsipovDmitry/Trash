#include <core/node.h>
#include <core/scenerootnode.h>

#include "nodeprivate.h"

#include "renderer.h"
#include "drawables.h"

#include <utils/frustum.h>

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

void NodePrivate::doUpdate(uint64_t, uint64_t)
{
}

void NodePrivate::doBeforeChangingTransformation()
{
    for (auto child : thisNode.children())
        child->m().doBeforeChangingTransformation();
}

void NodePrivate::doAfterChangingTransformation()
{
    for (auto child : thisNode.children())
        child->m().doAfterChangingTransformation();
}

Scene *NodePrivate::getScene() const
{
    Node *node = &thisNode;

    while (node->parent())
        node = node->parent();

    Scene *scene = nullptr;
    if (auto sceneRootNode = dynamic_cast<SceneRootNode*>(node))
        scene = sceneRootNode->scene();

    return scene;
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
