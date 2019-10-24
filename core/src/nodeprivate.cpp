#include <core/node.h>

#include "nodeprivate.h"

NodePrivate::NodePrivate(Node &node)
    : thisNode(node)
    , isGlobalTransformDirty(true)
{
}

NodePrivate::~NodePrivate()
{
}

void NodePrivate::dirtyGlobalTransform()
{
    isGlobalTransformDirty = true;
    for (auto child : thisNode.children())
        child->m().dirtyGlobalTransform();
}
