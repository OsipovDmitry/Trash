#include <queue>

#include <core/node.h>

#include "graphicscontrollerprivate.h"
#include "nodeprivate.h"

GraphicsControllerPrivate::GraphicsControllerPrivate(Renderer &r)
    : renderer(r)
{
    rootNode = std::make_shared<Node>();
}

void GraphicsControllerPrivate::updateScene(uint64_t time, uint64_t dt)
{
    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(rootNode);

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        node->m().doUpdate(renderer, time, dt);

        for (auto child : node->children())
            nodes.push(child);
    }
}
