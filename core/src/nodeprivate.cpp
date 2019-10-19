#include <QOpenGLExtraFunctions>

#include <glm/gtc/matrix_transform.hpp>

#include <core/core.h>
#include <core/graphicscontroller.h>
#include <core/node.h>

#include "graphicscontrollerprivate.h"
#include "nodeprivate.h"
#include "renderer.h"

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
