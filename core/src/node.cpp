#include <utils/transform.h>
#include <core/node.h>

#include "nodeprivate.h"

namespace trash
{
namespace core
{

Node::Node()
    : m_(std::make_unique<NodePrivate>(*this))
{
}

Node::Node(NodePrivate *nodePrivate)
    : m_(std::unique_ptr<NodePrivate>(nodePrivate))
{
}

Node::~Node()
{
}

bool Node::isDrawableNode() const
{
    return false;
}

bool Node::isModelNode() const
{
    return false;
}

void Node::setTransform(const utils::Transform& value)
{
    m_->dirtyShadowMaps(); // before transformation
    m_->transform = value;
    m_->dirtyGlobalTransform();
    m_->dirtyLightIndices();
    m_->dirtyShadowMaps(); // after transformation
    if (parent())
        parent()->m_->dirtyBoundingSphere();
}

const utils::Transform &Node::transform() const
{
    return m_->transform;
}

const utils::Transform &Node::globalTransform() const
{
    return m_->getGlobalTransform();
}

const utils::BoundingSphere &Node::boundingSphere() const
{
    return m_->getBoundingSphere();
}

std::shared_ptr<NodeUserData> Node::userData() const
{
    return m_->userData;
}

void Node::setUserData(std::shared_ptr<NodeUserData> data)
{
    m_->userData = data;
}

void Node::doAttach()
{
    m_->dirtyGlobalTransform();
    m_->dirtyLightIndices();
    m_->dirtyShadowMaps();
    if (parent())
        parent()->m_->dirtyBoundingSphere();
}

void Node::doDetach()
{
    m_->dirtyLightIndices();
    m_->dirtyShadowMaps();
    m_->dirtyGlobalTransform();
    if (parent())
        parent()->m_->dirtyBoundingSphere();
}

} // namespace
} // namespace
