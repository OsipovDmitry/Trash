#include <core/node.h>

#include "nodeprivate.h"
#include "sceneprivate.h"
#include "drawables.h"
#include "renderer.h"

namespace trash
{
namespace core
{

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

//        drawables.erase(bSphereDrawable);
//        bSphereDrawable = std::make_shared<SphereDrawable>(8, boundingSphere, glm::vec4(0.8f, .0f, .0f, 1.0f));
//        drawables.insert(bSphereDrawable);
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
