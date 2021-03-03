#include <queue>

#include <core/drawablenode.h>

#include "drawablenodeprivate.h"
#include "sceneprivate.h"
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

void DrawableNode::removeAllDrawables()
{
    m().removeAllDrawables();
}

void DrawableNode::setIntersectionMode(IntersectionMode mode)
{
    m().intersectionMode = mode;
}

IntersectionMode DrawableNode::intersectionMode() const
{
    return m().intersectionMode;
}

void DrawableNode::enableLighting(bool value)
{
    auto& nodePrivate = m();
    if (nodePrivate.lightIndices->isEnabled != value)
    {
        nodePrivate.lightIndices->isEnabled = value;
        nodePrivate.doDirtyLightIndices();
        nodePrivate.dirtyDrawables();
    }
}

bool DrawableNode::isLightingEnabled() const
{
    return m().lightIndices->isEnabled;
}

void DrawableNode::enableShadows(bool value)
{
    auto& nodePrivate = m();
    if (nodePrivate.areShadowsEnabled != value)
    {
        nodePrivate.areShadowsEnabled = value;
        nodePrivate.doDirtyShadowMaps();
    }
}

bool DrawableNode::areShadowsEnabled() const
{
    return m().areShadowsEnabled;
}

DrawableNode::DrawableNode(NodePrivate *nodePrivate)
    : Node(nodePrivate)
{
}

void DrawableNode::doAttach()
{
    Node::doAttach();
    ScenePrivate::dirtyNodeLightIndices(*this);
    ScenePrivate::dirtyNodeShadowMaps(*this);
}

void DrawableNode::doDetach()
{
    Node::doDetach();
    ScenePrivate::dirtyNodeLightIndices(*this);
    ScenePrivate::dirtyNodeShadowMaps(*this);
}


} // namespace
} // namespace
