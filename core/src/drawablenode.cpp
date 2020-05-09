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

DrawableNode *DrawableNode::asDrawableNode()
{
    return this;
}

const DrawableNode *DrawableNode::asDrawableNode() const
{
    return this;
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
