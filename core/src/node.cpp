#include <utils/transform.h>
#include <core/core.h>
#include <core/graphicscontroller.h>
#include <core/node.h>

#include "graphicscontrollerprivate.h"
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

void Node::doAttach()
{
    m_->isGlobalTransformDirty = true;
}

void Node::doDetach()
{
    m_->isGlobalTransformDirty = true;
}
