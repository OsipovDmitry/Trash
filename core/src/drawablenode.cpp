#include <queue>

#include <core/drawablenode.h>

#include "drawablenodeprivate.h"
#include "renderer.h"
#include "drawables.h"

namespace trash
{
namespace core
{

DrawableNode::DrawableNode()
    : Node(new DrawableNodePrivate(*this))
{

}

bool DrawableNode::isDrawableNode() const
{
    return true;
}

void DrawableNode::addDrawable(std::shared_ptr<Drawable> drawable)
{
    auto& dnPrivate = m();

    dnPrivate.drawables.insert(drawable);

    dnPrivate.dirtyBoundingSphere();
    dnPrivate.doDirtyLightIndices();
    dnPrivate.doDirtyShadowMaps();
}

void DrawableNode::removeDrawable(std::shared_ptr<Drawable> drawable)
{
    auto& dnPrivate = m();

    dnPrivate.drawables.erase(drawable);

    dnPrivate.dirtyBoundingSphere();
    dnPrivate.doDirtyLightIndices();
    dnPrivate.doDirtyShadowMaps();
}


} // namespace
} // namespace
