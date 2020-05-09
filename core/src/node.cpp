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

void Node::setTransform(const utils::Transform& value)
{
    m_->doBeforeChangingTransformation();
    m_->transform = value;
    m_->dirtyGlobalTransform();
    m_->dirtyBoundingBox();
    m_->doAfterChangingTransformation();
}

const utils::Transform &Node::transform() const
{
    return m_->transform;
}

const utils::Transform &Node::globalTransform() const
{
    return m_->getGlobalTransform();
}

const utils::BoundingBox &Node::boundingBox() const
{
    return m_->getBoundingBox();
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
    if (parent())
        parent()->m_->dirtyBoundingBox();
}

void Node::doDetach()
{
    m_->dirtyGlobalTransform();
    if (parent())
        parent()->m_->dirtyBoundingBox();
}

} // namespace
} // namespace
