#include <utils/transform.h>
#include <core/node.h>

#include "nodeprivate.h"

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

void Node::setTransform(const Transform& value)
{
    m_->transform = value;
    m_->dirtyGlobalTransform();
    if (parent())
        parent()->m_->dirtyBoundingSphere();
}

const Transform &Node::transform() const
{
    return m_->transform;
}

const Transform &Node::globalTransform() const
{
    if (m_->isGlobalTransformDirty)
    {
        m_->globalTransform =
                (m_parent ? m_parent->globalTransform() : Transform()) *
                m_->transform;
        m_->isGlobalTransformDirty = false;
    }

    return m_->globalTransform;
}

const BoundingSphere &Node::boundingSphere() const
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
    if (parent())
        parent()->m_->dirtyBoundingSphere();
}

void Node::doDetach()
{
    m_->dirtyGlobalTransform();
    if (parent())
        parent()->m_->dirtyBoundingSphere();
}
