#include <utils/ray.h>
#include <utils/frustum.h>

#include <core/nodeintersectionvisitor.h>
#include <core/node.h>
#include <core/drawablenode.h>

#include "nodeprivate.h"
#include "drawablenodeprivate.h"
#include "drawables.h"
#include "renderer.h"

namespace trash
{
namespace core
{

class NodeRayIntersectionVisitorPrivate
{
public:
    NodeRayIntersectionVisitorPrivate(const utils::Ray& r) : ray(r) {}
    ~NodeRayIntersectionVisitorPrivate();

    utils::Ray ray;
    IntersectionData data;
};

NodeRayIntersectionVisitorPrivate::~NodeRayIntersectionVisitorPrivate()
{
}

class NodeFrustumIntersectionVisitorPrivate
{
public:
    NodeFrustumIntersectionVisitorPrivate(const utils::Frustum& f) : frustum(f) {}
    ~NodeFrustumIntersectionVisitorPrivate();

    utils::Frustum frustum;
    IntersectionData data;
};


NodeFrustumIntersectionVisitorPrivate::~NodeFrustumIntersectionVisitorPrivate()
{
}

NodeRayIntersectionVisitor::NodeRayIntersectionVisitor(const utils::Ray& ray)
    : m_(std::make_unique<NodeRayIntersectionVisitorPrivate>(ray))
{
}

NodeRayIntersectionVisitor::~NodeRayIntersectionVisitor()
{
}

bool NodeRayIntersectionVisitor::visit(std::shared_ptr<Node> node)
{
    if (!m_->ray.intersect(node->globalTransform() * node->boundingBox()))
        return false;

    if (auto drawableNode = std::dynamic_pointer_cast<DrawableNode>(node))
    {
        float boundBoxT0, boundBoxT1;

        if (m_->ray.intersect(node->globalTransform() * node->m().getLocalBoundingBox(), &boundBoxT0, &boundBoxT1))
        {
            auto drawableIntersectionMode = drawableNode->intersectionMode();
            if (drawableIntersectionMode == IntersectionMode::None)
            {
                // do nothing
            }
            if (drawableIntersectionMode == IntersectionMode::UseBoundingBox)
            {
                m_->data.nodes.insert({glm::max(.0f, boundBoxT0), drawableNode});
                m_->data.nodes.insert({boundBoxT1, drawableNode});
            }
            else if (drawableIntersectionMode == IntersectionMode::UseGeometry)
            {
                auto nodeGLobalTransformInv = node->globalTransform().inverted();
                auto invertedRay = nodeGLobalTransformInv * m_->ray;
                auto dirRayScale = glm::length(m_->ray.dir * nodeGLobalTransformInv.scale);

                for (auto drawable : drawableNode->m().drawables)
                {
                    auto mesh = drawable->mesh();
                    float t0, t1;

                    if (!invertedRay.intersect(mesh->boundingBox, &t0, &t1))
                        continue;

                    auto verteBuffer = mesh->vertexBuffer(VertexAttribute::Position);
                    if (!verteBuffer)
                        continue;

                    assert(verteBuffer->numComponents == 2 || verteBuffer->numComponents == 3);

                    const void *vertexData = verteBuffer->cpuData();

                    for (auto indexBuffer : mesh->indexBuffers)
                    {
                        if (indexBuffer->primitiveType != GL_TRIANGLES)
                            continue;

                        const uint32_t *indexData = static_cast<const uint32_t*>(indexBuffer->cpuData());

                        std::set<float> rayCoords;
                        if (verteBuffer->numComponents == 3)
                            invertedRay.intersect(static_cast<const glm::vec3*>(vertexData), indexData, indexBuffer->numIndices, &rayCoords);
                        else if (verteBuffer->numComponents == 2)
                            invertedRay.intersect(static_cast<const glm::vec2*>(vertexData), indexData, indexBuffer->numIndices, &rayCoords);

                        for (auto t : rayCoords)
                            m_->data.nodes.insert({t / dirRayScale, drawableNode});
                    }
                }
            }
        }
    }

    return true;
}

const IntersectionData &NodeRayIntersectionVisitor::intersectionData() const
{
    return m_->data;
}

NodeFrustumIntersectionVisitor::NodeFrustumIntersectionVisitor(const utils::Frustum& frustum)
    : m_(std::make_unique<NodeFrustumIntersectionVisitorPrivate>(frustum))
{
}

NodeFrustumIntersectionVisitor::~NodeFrustumIntersectionVisitor()
{
}

bool NodeFrustumIntersectionVisitor::visit(std::shared_ptr<Node> node)
{
    if (!m_->frustum.contain(node->globalTransform() * node->boundingBox()))
        return false;

    if (auto drawableNode = std::dynamic_pointer_cast<DrawableNode>(node))
    {
        const auto box = node->globalTransform() * node->m().getLocalBoundingBox();
        if (m_->frustum.contain(box))
        {
            std::pair<float, float> distsToBox = box.pairDistancesToPlane(m_->frustum.planes.at(4));
            if (distsToBox.second > .0f)
            {
                distsToBox.first = std::max(.0f, distsToBox.first);
                m_->data.nodes.insert({distsToBox.first, drawableNode});
            }
        }
    }

    return true;
}

const IntersectionData &NodeFrustumIntersectionVisitor::intersectionData() const
{
    return m_->data;
}

} // namespace
} // namespace
