#include <queue>

#include <core/node.h>

#include "graphicscontrollerprivate.h"
#include "nodeprivate.h"

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
    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(rootNode);

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        node->m().doUpdate(time, dt);

        for (auto child : node->children())
            nodes.push(child);
    }
}
