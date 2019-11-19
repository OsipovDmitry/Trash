#include <queue>

#include <utils/frustum.h>
#include <core/node.h>

#include "graphicscontrollerprivate.h"
#include "nodeprivate.h"
#include "renderer.h"

GraphicsControllerPrivate::GraphicsControllerPrivate()
    : AbstractControllerPrivate()
{
    rootNode = std::make_shared<Node>();
}

GraphicsControllerPrivate::~GraphicsControllerPrivate()
{
}

void GraphicsControllerPrivate::updateScene(uint64_t time, uint64_t dt)
{
    auto& renderer = Renderer::instance();
    Frustum frustum(renderer.projectionMatrix() * viewMatrix);

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(rootNode);

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        if (!frustum.contain(node->globalTransform() * node->boundingSphere()))
            continue;

        node->m().doUpdate(time, dt);

        for (auto child : node->children())
            nodes.push(child);
    }
}
