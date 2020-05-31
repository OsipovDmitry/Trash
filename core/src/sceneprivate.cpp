#include <algorithm>
#include <queue>

#include <utils/frustum.h>
#include <utils/ray.h>

#include <core/scene.h>
#include <core/light.h>
#include <core/camera.h>
#include <core/drawablenode.h>

#include "renderer.h"
#include "drawables.h"
#include "cameraprivate.h"
#include "drawablenodeprivate.h"
#include "sceneprivate.h"
#include "lightprivate.h"

namespace trash
{
namespace core
{

const float ScenePrivate::CameraMinZNear = 500.0f;
const float ScenePrivate::ShadowMapMinZNear = 20.0f;
const int32_t ScenePrivate::ShadowMapSize = 512;

SceneRootNode::SceneRootNode(Scene *scene)
    : Node(new NodePrivate(*this))
    , m_scene(scene)
{
}

Scene *SceneRootNode::scene() const
{
    return m_scene;
}

ScenePrivate::ScenePrivate(Scene *scene)
    : thisScene(*scene)
    , rootNode(std::make_shared<SceneRootNode>(scene))
    , camera(std::make_shared<Camera>())
    , lights(std::make_shared<LightsList>())
    , backgroundDrawable(std::make_shared<BackgroundDrawable>())
{
}

void ScenePrivate::attachLight(std::shared_ptr<Light> light)
{
    auto& lightPrivate = light->m();
    if (lightPrivate.scene)
        lightPrivate.scene->detachLight(light);

    lightPrivate.scene = &thisScene;

    if (!freeLightIndices.empty())
    {
        auto freeIndexIt = freeLightIndices.begin();
        auto index = *freeIndexIt;
        freeLightIndices.erase(freeIndexIt);
        lights->at(static_cast<size_t>(index)) = light;
        dirtyLights.insert(index);
        dirtyShadowMaps.insert(index);

    }
    else
    {
        lights->push_back(light);
        auto numLights = lights->size();
        lights->resize(numLights+32);
        for (size_t i = 0; i < 32; ++i)
            freeLightIndices.insert(numLights+i);

        for (size_t i = 0; i < numLights; ++i)
        {
            dirtyLights.insert(i);
            dirtyShadowMaps.insert(i);
        }

        size_t bufferSize = 2 * sizeof(glm::mat4x4) * lights->size();
        lightsUbo = std::make_shared<Buffer>(bufferSize, nullptr, GL_STATIC_DRAW);

        auto& renderer = Renderer::instance();
        lightsShadowMaps = renderer.createTexture2DArray(GL_DEPTH_COMPONENT16, ShadowMapSize, ShadowMapSize, static_cast<GLint>(lights->size()), GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        lightsShadowMaps->setWrap(GL_CLAMP_TO_BORDER);
        lightsShadowMaps->setBorderColor(glm::vec4(1.f, 1.f, 1.f, 1.f));
        lightsShadowMaps->setCompareMode(GL_COMPARE_REF_TO_TEXTURE);
        lightsShadowMaps->setCompareFunc(GL_LEQUAL);
    }

    ScenePrivate::dirtyNodeLightIndices(*rootNode);
}

bool ScenePrivate::detachLight(std::shared_ptr<Light> light)
{
    auto& lightPrivate = light->m();

    if (&thisScene != lightPrivate.scene)
        return false;

    auto lightIter = std::find(lights->begin(), lights->end(), light);
    freeLightIndices.insert(std::distance(lights->begin(), lightIter));
    *lightIter = nullptr;
    lightPrivate.scene = nullptr;

    ScenePrivate::dirtyNodeLightIndices(*rootNode);
    return true;
}

void ScenePrivate::dirtyLightParams(Light *light)
{
    dirtyLights.insert(static_cast<uint32_t>(std::distance(
                                                 lights->begin(),
                                                 std::find_if(lights->begin(), lights->end(),
                                                              [light](const std::shared_ptr<trash::core::Light>& l){ return l.get() == light; }))
                                             ));
}

std::shared_ptr<Buffer> ScenePrivate::getLightParamsBuffer()
{
    updateLightParams();
    return lightsUbo;
}

void ScenePrivate::updateLightParams()
{
    static const glm::mat4x4 biasMatrix(0.5f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 0.5f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.5f, 0.0f,
                                        0.5f, 0.5f, 0.5f, 1.0f);

    for (auto lightIdx : dirtyLights)
    {
        auto light = lights->at(lightIdx);
        if (light)
        {
            auto dataOffset = 2 * lightIdx * sizeof(glm::mat4x4);
            auto dataSize = 2 * sizeof(glm::mat4x4);
            auto *p = reinterpret_cast<glm::mat4x4*>(lightsUbo->map(dataOffset, dataSize, GL_MAP_WRITE_BIT));
            p[0] = light->m().packParams();
            p[1] = biasMatrix * light->m().getMatrix();
            lightsUbo->unmap();
        }
    }

    dirtyLights.clear();
}

void ScenePrivate::dirtyShadowMap(Light *light)
{
    dirtyShadowMaps.insert(static_cast<uint32_t>(std::distance(
                                                     lights->begin(),
                                                     std::find_if(lights->begin(), lights->end(),
                                                                  [light](const std::shared_ptr<trash::core::Light>& l){ return l.get() == light; }))
                                                 ));
}

std::shared_ptr<Texture> ScenePrivate::getLightsShadowMaps()
{
    updateShadowMaps();
    return lightsShadowMaps;
}

void ScenePrivate::updateShadowMaps()
{   
    for (auto lightIdx : dirtyShadowMaps)
    {
        auto light = lights->at(lightIdx);
        if (light)
        {
            light->m().attachShadowMap(lightsShadowMaps, static_cast<uint32_t>(lightIdx));
            updateShadowMap(light);
        }
    }
    dirtyShadowMaps.clear();
}

void ScenePrivate::updateShadowMap(std::shared_ptr<Light> light)
{
    if (!light->isShadowMapEnabled())
        return;

    auto& lightPrivate = light->m();
    lightPrivate.setZPlanes(calculateZPlanes(lightPrivate.calcMatrix({0.f, 1.f}), ShadowMapMinZNear));
    utils::Frustum frustum(lightPrivate.getMatrix());

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(rootNode);

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        if (!frustum.contain(node->globalTransform() * node->boundingBox()))
            continue;

        if (auto drawableNode = node->asDrawableNode())
            drawableNode->m().doUpdateShadowMaps();

        for (auto child : node->children())
            nodes.push(child);
    }

    auto& renderer = Renderer::instance();
    renderer.setViewport(glm::ivec4(0, 0, ShadowMapSize, ShadowMapSize));
    renderer.setViewMatrix(glm::mat4x4(1.0f));
    renderer.setProjectionMatrix(lightPrivate.getMatrix());
    renderer.setClearColor(false);
    renderer.setClearDepth(true, 1.0f);
    renderer.setLightsBuffer(nullptr);
    renderer.setShadowMaps(nullptr);
    renderer.render(lightPrivate.shadowMapFramebuffer);
}

void ScenePrivate::dirtyNodeLightIndices(Node& dirtyNode)
{
    std::queue<Node*> nodes;
    nodes.push(&dirtyNode);

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();
        if (auto drawableNode = node->asDrawableNode())
            drawableNode->m().doDirtyLightIndices();

        for (auto child : node->children())
            nodes.push(child.get());
    }
}

void ScenePrivate::dirtyNodeShadowMaps(Node& dirtyNode)
{
    std::queue<Node*> nodes;
    nodes.push(&dirtyNode);

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();
        if (auto drawableNode = node->asDrawableNode())
            drawableNode->m().doDirtyShadowMaps();

        for (auto child : node->children())
            nodes.push(child.get());
    }
}

void ScenePrivate::renderScene(uint64_t time, uint64_t dt)
{
    auto& cameraPrivate = camera->m();

    cameraPrivate.setZPlanes(calculateZPlanes(cameraPrivate.calcProjectionMatrix({0.f, 1.f}) * cameraPrivate.getViewMatrix(), CameraMinZNear));
    utils::Frustum frustum(cameraPrivate.getProjectionMatrix() * cameraPrivate.getViewMatrix());

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(rootNode);

    // TODO: update only lights that are used in this frame
    auto shadowMaps = getLightsShadowMaps();

    auto& renderer = Renderer::instance();

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        if (!frustum.contain(node->globalTransform() * node->boundingBox()))
            continue;

        node->m().doUpdate(time, dt);

        for (auto child : node->children())
            nodes.push(child);
    }

    for (auto l : *lights)
    {
        if (l)
        {
//            renderer.draw(
//                    std::make_shared<SphereDrawable>(2, utils::BoundingSphere(glm::vec3(), 10.0f), glm::vec4(l->color(),1)),
//                    utils::Transform(glm::vec3(1,1,1), glm::quat(1,0,0,0), l->position()));

            //renderer.draw(std::make_shared<FrustumDrawable>(utils::Frustum(l->m().getMatrix()), glm::vec4(l->color(),1)), utils::Transform());
        }
    }

    renderer.setViewport(cameraPrivate.viewport);
    renderer.setViewMatrix(cameraPrivate.getViewMatrix());
    renderer.setProjectionMatrix(cameraPrivate.getProjectionMatrix());
    renderer.setClearColor(cameraPrivate.clearColorBuffer, cameraPrivate.clearColor);
    renderer.setClearDepth(cameraPrivate.clearDepthBuffer, cameraPrivate.clearDepth);
    renderer.setIBLMaps(renderer.loadTexture("textures/diffuse/diffuse.json"), renderer.loadTexture("textures/specular/specular.json"));
    renderer.setLightsBuffer(getLightParamsBuffer());
    renderer.setShadowMaps(shadowMaps);

    renderer.draw(backgroundDrawable, utils::Transform());
    renderer.render(nullptr);

//    auto f = renderer.functions();
//    f.glBindFramebuffer(GL_READ_FRAMEBUFFER, lights->at(2)->m().shadowMapFramebuffer->id);
//    f.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer.defaultFbo());
//    f.glBlitFramebuffer(0,0,1024,1024,0,0,1024,1024,GL_COLOR_BUFFER_BIT,GL_NEAREST);

}

PickData ScenePrivate::pickScene(int32_t xi, int32_t yi)
{
    static uint32_t backgroundId = 0xFFFFFFFF;

    auto& cameraPrivate = camera->m();
    auto& renderer = Renderer::instance();
    renderer.setViewport(cameraPrivate.viewport);
    renderer.setViewMatrix(cameraPrivate.getProjectionMatrix());
    renderer.setViewMatrix(cameraPrivate.getViewMatrix());
    renderer.setClearColor(true, SelectionDrawable::idToColor(backgroundId));
    renderer.setClearDepth(true, 1.0f);
    renderer.setLightsBuffer(nullptr);
    renderer.setShadowMaps(nullptr);

    const float x = static_cast<float>(xi - cameraPrivate.viewport.x) / static_cast<float>(cameraPrivate.viewport.z) * 2.0f - 1.0f;
    const float y = (1.0f - static_cast<float>(yi - cameraPrivate.viewport.y) / static_cast<float>(cameraPrivate.viewport.w)) * 2.0f - 1.0f;

    auto viewProjectionMatrixInverse = glm::inverse(cameraPrivate.getProjectionMatrix() * cameraPrivate.getViewMatrix());

    glm::vec4 p0 = viewProjectionMatrixInverse * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 p1 = viewProjectionMatrixInverse * glm::vec4(x, y, 1.0f, 1.0f);

    p0 /= p0.w;
    p1 /= p1.w;

    utils::Ray ray(p0, p1-p0);

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(rootNode);

    std::vector<std::shared_ptr<Node>> nodeIds;

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        if (!ray.intersect(node->globalTransform() * node->boundingBox()))
            continue;

        node->m().doPick(static_cast<uint32_t>(nodeIds.size()));
        nodeIds.push_back(node);

        for (auto child : node->children())
            nodes.push(child);
    }

    auto pickBuffer = std::make_shared<Framebuffer>();
    pickBuffer->attachColor(std::make_shared<Renderbuffer>(GL_RGBA8, cameraPrivate.viewport.z, cameraPrivate.viewport.w));
    pickBuffer->attachDepth(std::make_shared<Renderbuffer>(GL_DEPTH_COMPONENT32, cameraPrivate.viewport.z, cameraPrivate.viewport.w));

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

IntersectionData ScenePrivate::intersectScene(const utils::Ray& ray)
{
    IntersectionData resultData;

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(rootNode);

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        if (!ray.intersect(node->globalTransform() * node->boundingBox()))
            continue;

        if (auto drawableNode = node->asDrawableNode())
        {
            float boundBoxT0, boundBoxT1;

            if (!ray.intersect(node->globalTransform() * node->m().getLocalBoundingBox(), &boundBoxT0, &boundBoxT1))
                continue;

            auto drawableIntersectionMode = drawableNode->intersectionMode();
            if (drawableIntersectionMode == IntersectionMode::UseBoundingBox)
            {
                resultData.nodes.insert({glm::max(.0f, boundBoxT0), node});
                resultData.nodes.insert({boundBoxT1, node});
            }
            else if (drawableIntersectionMode == IntersectionMode::UseGeometry)
            {
                for (auto drawable : drawableNode->m().drawables)
                {
                    auto mesh = drawable->mesh();
                    float t0, t1;

                    if (!ray.intersect(node->globalTransform() * mesh->boundingBox, &t0, &t1))
                        continue;

                    auto verteBuffer = mesh->vertexBuffer(VertexAttribute::Position);
                    if (!verteBuffer)
                        continue;

                    assert(verteBuffer->numComponents == 2 || verteBuffer->numComponents == 3);

                    const void *vertexData = verteBuffer->cpuData();
                    const glm::vec3 *vertexDataAsVec3 = static_cast<const glm::vec3*>(vertexData);
                    const glm::vec2 *vertexDataAsVec2 = static_cast<const glm::vec2*>(vertexData);

                    for (auto indexBuffer : mesh->indexBuffers)
                    {
                        if (indexBuffer->primitiveType != GL_TRIANGLES)
                            continue;

                        const uint32_t *indexData = static_cast<const uint32_t*>(indexBuffer->cpuData());

                        glm::vec2 barycentric;
                        float t;

                        for (uint32_t i = 0; i < indexBuffer->numIndices; i += 3)
                        {
                            if (verteBuffer->numComponents == 3)
                            {
                                if (glm::intersectRayTriangle(ray.pos, ray.dir,
                                                              vertexDataAsVec3[indexData[i]],
                                                              vertexDataAsVec3[indexData[i+1]],
                                                              vertexDataAsVec3[indexData[i+2]],
                                                              barycentric, t))
                                    resultData.nodes.insert({t, node});
                            }
                            else if (verteBuffer->numComponents == 2)
                            {
                                if (glm::intersectRayTriangle(ray.pos, ray.dir,
                                                              glm::vec3(vertexDataAsVec2[indexData[i]], .0f),
                                                              glm::vec3(vertexDataAsVec2[indexData[i+1]], .0f),
                                                              glm::vec3(vertexDataAsVec2[indexData[i+2]], .0f),
                                                              barycentric, t))
                                    resultData.nodes.insert({t, node});
                            }
                        }

                    }
                }
            }
        }

        for (auto child : node->children())
            nodes.push(child);
    }

    return resultData;
}

IntersectionData ScenePrivate::intersectScene(const utils::Frustum& frustum)
{
    IntersectionData resultData;

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(rootNode);

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        if (!frustum.contain(node->globalTransform() * node->boundingBox()))
            continue;

        if (/*auto drawableNode = */node->asDrawableNode())
        {
            const auto box = node->globalTransform() * node->m().getLocalBoundingBox();
            if (frustum.contain(box))
            {
                std::pair<float, float> distsToBox;
                box.distanceToPlane(frustum.planes.at(4), distsToBox);
                if (distsToBox.second > .0f)
                {
                    distsToBox.first = std::max(.0f, distsToBox.first);
                    resultData.nodes.insert({distsToBox.first, node});
                }
            }
        }

        for (auto child : node->children())
            nodes.push(child);
    }

    return resultData;
}

std::pair<float, float> ScenePrivate::calculateZPlanes(const glm::mat4x4& viewProjMatrix, float minZNear) const
{
    utils::OpenFrustum openFrustum(viewProjMatrix);
    float zNear = +FLT_MAX, zFar = -FLT_MAX;

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(rootNode);

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        if (!openFrustum.contain(node->globalTransform() * node->boundingBox()))
            continue;

        if (/*auto drawableNode = */node->asDrawableNode())
        {
            const auto box = node->globalTransform() * node->m().getLocalBoundingBox();
            if (openFrustum.contain(box))
            {
                std::pair<float, float> distsToBox;
                box.distanceToPlane(openFrustum.planes.at(4), distsToBox);
                if (distsToBox.second > .0f)
                {
                    distsToBox.first = std::max(.0f, distsToBox.first);
                    zNear = glm::min(zNear, distsToBox.first);
                    zFar = glm::max(zFar, distsToBox.second);
                }
            }
        }

        for (auto child : node->children())
            nodes.push(child);
    }

    return { glm::max(zNear, minZNear), zFar };
}

} // namespace
} // namespace
