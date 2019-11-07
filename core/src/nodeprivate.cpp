#include <core/node.h>

#include "nodeprivate.h"
#include "renderer.h"

NodePrivate::NodePrivate(Node &node)
    : thisNode(node)
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
    while (thisNode.parent())
        thisNode.parent()->m().dirtyBoundingSphere();
}

std::shared_ptr<Drawable> NodePrivate::boundingSphereDrawable()
{
    thisNode.boundingSphere();
    return bSphereDrawable;
}

void NodePrivate::doUpdate(uint64_t, uint64_t)
{
    Renderer::instance().draw(0, boundingSphereDrawable(), thisNode.globalTransform());
}
