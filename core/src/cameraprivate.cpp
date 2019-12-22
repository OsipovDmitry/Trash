#include <queue>

#include <core/node.h>
#include <core/scene.h>
#include <core/camera.h>
#include <utils/frustum.h>
#include <utils/ray.h>

#include "cameraprivate.h"
#include "renderer.h"
#include "nodeprivate.h"
#include "drawables.h"

namespace trash
{
namespace core
{

const glm::mat4x4 &CameraPrivate::viewMatrix() const
{
    return viewMatrixCache;
}

const glm::mat4x4 &CameraPrivate::projectionMatrix() const
{
    if (projMatrixDirty)
    {
        const float aspect = static_cast<float>(viewport.z) / static_cast<float>(viewport.w);

        projMatrixCache = projMatrixAsOrtho ?
                    glm::ortho(-aspect, +aspect, -1.0f, +1.0f, zNear, zFar) :
                    glm::perspective(fov, aspect, zNear, zFar);
    }
    return projMatrixCache;
}

void CameraPrivate::renderScene(uint64_t time, uint64_t dt)
{
    auto& renderer = Renderer::instance();
    renderer.setViewport(viewport);
    renderer.setViewMatrix(viewMatrix());
    renderer.setProjectionMatrix(projectionMatrix());
    renderer.setClearColor(clearColorBuffer, clearColor);
    renderer.setClearDepth(clearDepthBuffer, clearDepth);

    utils::Frustum frustum(projectionMatrix() * viewMatrix());

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(scene->rootNode());

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

    renderer.render(nullptr);
}

PickData CameraPrivate::pickNode(int32_t xi, int32_t yi)
{
    static uint32_t backgroundId = 0xFFFFFFFF;

    auto& renderer = Renderer::instance();
    renderer.setViewport(viewport);
    renderer.setViewMatrix(viewMatrix());
    renderer.setClearColor(true, SelectionDrawable::idToColor(backgroundId));
    renderer.setClearDepth(true, 1.0f);

    const float x = static_cast<float>(xi - viewport.x) / static_cast<float>(viewport.z) * 2.0f - 1.0f;
    const float y = (1.0f - static_cast<float>(yi - viewport.y) / static_cast<float>(viewport.w)) * 2.0f - 1.0f;

    auto viewProjectionMatrixInverse = glm::inverse(projectionMatrix() * viewMatrix());

    glm::vec4 p0 = viewProjectionMatrixInverse * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 p1 = viewProjectionMatrixInverse * glm::vec4(x, y, 1.0f, 1.0f);

    p0 /= p0.w;
    p1 /= p1.w;

    utils::Ray ray(p0, p1-p0);

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(scene->rootNode());

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

    auto pickBuffer = std::make_shared<Framebuffer>(GL_RGBA8);
    pickBuffer->resize(viewport.z, viewport.w);

    renderer.render(pickBuffer);

    uint8_t color[4];
    float depth;
    renderer.readPixel(pickBuffer, xi, yi, color[0], color[1], color[2], color[3], depth);
    uint32_t id = SelectionDrawable::colorToId(color[0], color[1], color[2], color[3]);

    std::shared_ptr<Node> node = (id != 0xFFFFFFFF) ? nodeIds[id] : nullptr;

    depth = depth * 2.0f - 1.0f;
    p0 = viewProjectionMatrixInverse * glm::vec4(x, y, depth, 1.0f);
    p0 /= p0.w;

    glm::vec3 localCoord(0.0f, 0.0f, 0.0f);
    if (node)
        localCoord = node->globalTransform().inverse() * glm::vec3(p0.x, p0.y, p0.z);

    return PickData{node, localCoord};
}

} // namespace
} // namespace
