#ifndef NODEINTERSECTIONVISITOR_H
#define NODEINTERSECTIONVISITOR_H

#include <vector>

#include <utils/ray.h>

#include <core/nodevisitor.h>
#include <core/node.h>
#include <core/drawablenode.h>

#include "drawablenodeprivate.h"

namespace trash
{
namespace core
{

class NodePickVisitor : public NodeVisitor
{
public:
    NodePickVisitor(const utils::Ray& ray) : m_ray(ray), m_nodeIds() {}

    bool visit(std::shared_ptr<Node> node) override
    {
        if (!m_ray.intersect(node->globalTransform() * node->boundingBox()))
            return false;

        if (auto drawableNode = std::dynamic_pointer_cast<DrawableNode>(node))
        {
            auto& drawableNodePrivate = drawableNode->m();
            if (m_ray.intersect(drawableNode->globalTransform() * drawableNodePrivate.getLocalBoundingBox()))
            {
                m_nodeIds.push_back(drawableNode);
                drawableNode->m().doRender(static_cast<uint32_t>(m_nodeIds.size()));
            }
        }

        return true;
    }

    const std::vector<std::shared_ptr<DrawableNode>>& nodeIds() const { return m_nodeIds; }

private:
    utils::Ray m_ray;
    std::vector<std::shared_ptr<DrawableNode>> m_nodeIds;

};

} // namespace
} // namespace

#endif // NODEINTERSECTIONVISITOR_H
