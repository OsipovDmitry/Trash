#include <core/node.h>

#include "nodeprivate.h"
#include "drawables.h"
#include "renderer.h"

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

void NodePrivate::doUpdate(uint64_t, uint64_t)
{
    auto& renderer = Renderer::instance();

    for (auto& drawable : drawables)
        renderer.draw(drawable, thisNode.globalTransform());
}

void NodePrivate::doPick(uint32_t id)
{
    auto& renderer = Renderer::instance();

    for (auto& drawable : drawables)
        renderer.draw(drawable->selectionDrawable(id), thisNode.globalTransform());
}

const BoundingSphere &NodePrivate::getBoundingSphere()
{
    if (isBoundingSphereDirty)
    {
        boundingSphere = minimalBoundingSphere;
        for (auto drawable : drawables)
            boundingSphere += drawable->mesh()->boundingSphere;
        for (auto child : thisNode.children())
            boundingSphere += child->transform() * child->boundingSphere();
        isBoundingSphereDirty = false;

//        drawables.erase(bSphereDrawable);
//        bSphereDrawable = std::make_shared<SphereDrawable>(8, boundingSphere, glm::vec4(0.8f, .0f, .0f, 1.0f));
//        drawables.insert(bSphereDrawable);
    }
    return boundingSphere;
}

void NodePrivate::addDrawable(std::shared_ptr<Drawable> drawable)
{
    drawables.insert(drawable);
    dirtyBoundingSphere();
}

void NodePrivate::removeDrawable(std::shared_ptr<Drawable> drawable)
{
    drawables.erase(drawable);
    dirtyBoundingSphere();
}
