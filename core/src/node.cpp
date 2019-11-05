#include <utils/transform.h>
#include <core/node.h>

#include "nodeprivate.h"
#include "renderer.h"

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
    if (m_->isBoundingSphereDirty)
    {
        m_->boundingSphere = m_->calcLocalBoundingSphere();
        for (auto child : children())
            m_->boundingSphere += child->transform() * child->boundingSphere();

        m_->bSphereDrawable = Renderer::instance().createSphereDrawable(8, m_->boundingSphere, glm::vec4(1.0f, .0f, .0f, 1.0f));

        m_->isBoundingSphereDirty = false;   
    }

    return m_->boundingSphere;
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
