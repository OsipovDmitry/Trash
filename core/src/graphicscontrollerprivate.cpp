#include <queue>

#include <utils/frustum.h>
#include <utils/ray.h>

#include <core/node.h>
#include <core/graphicscontroller.h>

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

PickData GraphicsControllerPrivate::pickNode(int32_t xi, int32_t yi)
{
    auto& renderer = Renderer::instance();
    const auto& vp = renderer.viewport();

    const float x = static_cast<float>(xi - vp.x) / static_cast<float>(vp.z) * 2.0f - 1.0f;
    const float y = (1.0f - static_cast<float>(yi - vp.y) / static_cast<float>(vp.w)) * 2.0f - 1.0f;

    auto viewProjectionMatrixInverse = glm::inverse(renderer.projectionMatrix() * viewMatrix);

    glm::vec4 p0 = viewProjectionMatrixInverse * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 p1 = viewProjectionMatrixInverse * glm::vec4(x, y, 1.0f, 1.0f);

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

    uint32_t idColor;
    float depth;
    renderer.pick(xi, yi, SelectionDrawable::idToColor(0xFFFFFFFF), idColor, depth);
    std::shared_ptr<Node> node = (idColor != 0xFFFFFFFF) ? nodeIds[idColor] : nullptr;

    depth = depth * 2.0f - 1.0f;
    p0 = viewProjectionMatrixInverse * glm::vec4(x, y, depth, 1.0f);
    p0 /= p0.w;

    glm::vec3 localCoord(0.0f, 0.0f, 0.0f);
    if (node)
        localCoord = node->globalTransform().inverse() * glm::vec3(p0.x, p0.y, p0.z);


    return PickData{node, localCoord};
}
