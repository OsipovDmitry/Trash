#include <queue>

#include <utils/frustum.h>
#include <utils/ray.h>

#include <core/node.h>

#include "graphicscontrollerprivate.h"
#include "nodeprivate.h"
#include "renderer.h"
#include "drawables.h"

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

std::shared_ptr<Node> GraphicsControllerPrivate::pickNode(int32_t xi, int32_t yi)
{
    auto& renderer = Renderer::instance();
    const auto& vp = renderer.viewport();

    const float x = static_cast<float>(xi - vp.x) / static_cast<float>(vp.z) * 2.0f - 1.0f;
    const float y = (1.0f - static_cast<float>(yi - vp.y) / static_cast<float>(vp.w)) * 2.0f - 1.0f;

    auto modelViewMatrixInverse = glm::inverse(renderer.projectionMatrix() * viewMatrix);

    glm::vec4 p0 = modelViewMatrixInverse * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 p1 = modelViewMatrixInverse * glm::vec4(x, y, 1.0f, 1.0f);

    p0 /= p0.w;
    p1 /= p1.w;

    Ray ray(p0, p1-p0);

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(rootNode);

    std::vector<std::shared_ptr<Node>> nodeIds;

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        if (!ray.intersect(node->globalTransform() * node->boundingSphere()))
            continue;

        node->m().doPick(static_cast<uint32_t>(nodeIds.size()));
        nodeIds.push_back(node);

        for (auto child : node->children())
            nodes.push(child);
    }

    uint32_t id = renderer.pick(xi, yi, SelectionDrawable::idToColor(0xFFFFFFFF));
    return (id != 0xFFFFFFFF) ? nodeIds[id] : nullptr;
}
