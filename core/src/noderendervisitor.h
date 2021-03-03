#ifndef NODERENDERVISITOR_H
#define NODERENDERVISITOR_H

#include <utils/frustum.h>

#include <core/nodevisitor.h>
#include <core/node.h>
#include <core/drawablenode.h>

#include "drawablenodeprivate.h"

namespace trash
{
namespace core
{

class NodeRenderVisitor : public NodeVisitor
{
public:
    NodeRenderVisitor(const utils::Frustum& cameraFrustum) : m_cameraFrustum(cameraFrustum) {}

    bool visit(std::shared_ptr<Node> node) override {
        if (!m_cameraFrustum.contain(node->globalTransform() * node->boundingBox()))
            return false;

        if (auto drawableNode = std::dynamic_pointer_cast<DrawableNode>(node))
        {
            auto& drawableNodePrivate = drawableNode->m();
            if (m_cameraFrustum.contain(drawableNode->globalTransform() * drawableNodePrivate.getLocalBoundingBox()))
                drawableNodePrivate.doRender(0);
        }

        return true;
    }

private:
    utils::Frustum m_cameraFrustum;

};

} // namespace
} // namespace

#endif // NODERENDERVISITOR_H
