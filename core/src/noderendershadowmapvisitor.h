#ifndef NODERENDERSHADOWMAPVISITOR_H
#define NODERENDERSHADOWMAPVISITOR_H

#include <utils/frustum.h>

#include <core/nodevisitor.h>
#include <core/node.h>
#include <core/drawablenode.h>

#include "drawablenodeprivate.h"

namespace trash
{
namespace core
{

class NodeRenderShadowMapVisitor : public NodeVisitor
{
public:
    NodeRenderShadowMapVisitor(const utils::Frustum& lightFrustum) : m_lightFrustum(lightFrustum) {}

    bool visit(std::shared_ptr<Node> node) override {
        if (!m_lightFrustum.contain(node->globalTransform() * node->boundingBox()))
            return false;

        if (auto drawableNode = std::dynamic_pointer_cast<DrawableNode>(node))
        {
            auto& drawableNodePrivate = drawableNode->m();
            if (drawableNode->areShadowsEnabled() &&
                m_lightFrustum.contain(drawableNode->globalTransform() * drawableNodePrivate.getLocalBoundingBox()))
                drawableNodePrivate.doRender(0);
        }

        return true;
    }

private:
    utils::Frustum m_lightFrustum;
};

} // namespace
} // namespace

#endif // NODERENDERSHADOWMAPVISITOR_H
