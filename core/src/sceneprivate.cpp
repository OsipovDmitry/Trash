#include <algorithm>
#include <queue>

#include <utils/frustum.h>
#include <utils/ray.h>

#include <core/light.h>
#include <core/camera.h>

#include "renderer.h"
#include "drawables.h"
#include "cameraprivate.h"
#include "sceneprivate.h"
#include "lightprivate.h"

#include <QtGui/QOpenGLExtraFunctions>

namespace trash
{
namespace core
{

const float ScenePrivate::ShadowMapZNear = 20.0f;
const float ScenePrivate::ShadowMapZFar = 3000.0f;
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
    : rootNode(std::make_shared<SceneRootNode>(scene))
    , lights(std::make_shared<LightsList>())
    , allLightsAreDirty(true)
    , allShadowMapsAreDirty(true)
{
}

void ScenePrivate::dirtyLightParams(Light *light)
{
    if (light == nullptr)
        allLightsAreDirty = true;
    else
        dirtyLights.insert(static_cast<uint32_t>(std::distance(
                                                     lights->begin(),
                                                     std::find_if(lights->begin(), lights->end(),
                                                                  [light](const std::shared_ptr<trash::core::Light>& l){ return l.get() == light; }))
                           ));
}

std::shared_ptr<Buffer> ScenePrivate::getLightParamsBuffer()
{
    if (allLightsAreDirty || !dirtyLights.empty())
        updateLightParams();

    return lightsUbo;
}

void ScenePrivate::updateLightParams()
{
    static const glm::mat4x4 biasMatrix(0.5f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 0.5f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.5f, 0.0f,
                                        0.5f, 0.5f, 0.5f, 1.0f);

    if (allLightsAreDirty)
    {
        size_t bufferSize = 2 * sizeof(glm::mat4x4) * lights->size();
        lightsUbo = std::make_shared<Buffer>(bufferSize, nullptr, GL_STATIC_DRAW);
        auto *p = reinterpret_cast<glm::mat4x4*>(lightsUbo->map(0, static_cast<GLsizeiptr>(bufferSize), GL_MAP_WRITE_BIT));
        for (size_t i = 0; i < lights->size(); ++i)
        {
            p[2*i+0] = lights->at(i)->m().packParams();
            p[2*i+1] = biasMatrix * lights->at(i)->m().matrix(ShadowMapZNear, ShadowMapZFar);

        }
        lightsUbo->unmap();
    }
    else
    {
        for (auto lightIdx : dirtyLights)
        {
            auto dataOffset = 2 * lightIdx * sizeof(glm::mat4x4);
            auto dataSize = 2 * sizeof(glm::mat4x4);
            auto *p = reinterpret_cast<glm::mat4x4*>(lightsUbo->map(dataOffset, dataSize, GL_MAP_WRITE_BIT));
            p[0] = lights->at(lightIdx)->m().packParams();
            p[1] = biasMatrix * lights->at(lightIdx)->m().matrix(ShadowMapZNear, ShadowMapZFar);
            lightsUbo->unmap();
        }
    }

    allLightsAreDirty = false;
    dirtyLights.clear();
}

void ScenePrivate::dirtyShadowMap(Light *light)
{
    if (light == nullptr)
        allShadowMapsAreDirty = true;
    else
        dirtyShadowMaps.insert(static_cast<uint32_t>(std::distance(
                                                     lights->begin(),
                                                     std::find_if(lights->begin(), lights->end(),
                                                                  [light](const std::shared_ptr<trash::core::Light>& l){ return l.get() == light; }))
                              ));
}

std::shared_ptr<Texture> ScenePrivate::getLightsShadowMaps()
{
    if (allShadowMapsAreDirty || !dirtyShadowMaps.empty())
        updateShadowMaps();

    return lightsShadowMaps;
}

void ScenePrivate::updateShadowMaps()
{
    auto& renderer = Renderer::instance();
    if (allShadowMapsAreDirty)
    {
        lightsShadowMaps = renderer.createTexture2DArray(GL_DEPTH_COMPONENT16, ShadowMapSize, ShadowMapSize, static_cast<GLint>(lights->size()), GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        lightsShadowMaps->setWrap(GL_CLAMP_TO_BORDER);
        lightsShadowMaps->setBorderColor(glm::vec4(1.f, 1.f, 1.f, 1.f));
        lightsShadowMaps->setCompareMode(GL_COMPARE_REF_TO_TEXTURE);
        lightsShadowMaps->setCompareFunc(GL_LEQUAL);

        for (size_t lightIdx = 0; lightIdx < lights->size(); ++lightIdx)
        {
            auto light = lights->at(lightIdx);
            light->m().attachShadowMap(lightsShadowMaps, static_cast<uint32_t>(lightIdx));
            updateShadowMap(light);
        }
    }
    else
    {
        for (auto lightIdx : dirtyShadowMaps)
        {
            auto light = lights->at(lightIdx);
            light->m().attachShadowMap(lightsShadowMaps, static_cast<uint32_t>(lightIdx));
            updateShadowMap(light);
        }
    }

    allShadowMapsAreDirty = false;
    dirtyShadowMaps.clear();
}

void ScenePrivate::updateShadowMap(std::shared_ptr<Light> light)
{
    if (!light->isShadowMapEnabled())
        return;

    auto& lightPrivate = light->m();

    auto lightMatrix = lightPrivate.matrix(ShadowMapZNear, ShadowMapZFar);
    utils::Frustum frustum(lightMatrix);

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(rootNode);

    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        if (!frustum.contain(node->globalTransform() * node->boundingSphere()))
            continue;

        node->m().doUpdateShadowMaps();

        for (auto child : node->children())
            nodes.push(child);
    }

    auto& renderer = Renderer::instance();
    renderer.setViewport(glm::ivec4(0, 0, ShadowMapSize, ShadowMapSize));
    renderer.setViewMatrix(glm::mat4x4(1.0f));
    renderer.setProjectionMatrix(lightMatrix);
    renderer.setClearColor(false);
    renderer.setClearDepth(true, 1.0f);
    renderer.setLightsBuffer(nullptr);
    renderer.setShadowMaps(nullptr);
    renderer.render(lightPrivate.shadowMapFramebuffer);
}

void ScenePrivate::renderScene(uint64_t time, uint64_t dt, const CameraPrivate& cameraPrivate)
{
    utils::Frustum frustum(cameraPrivate.projectionMatrix() * cameraPrivate.viewMatrix());

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(rootNode);

    // TODO: update only lights that are used in this frame
    auto shadowMaps = getLightsShadowMaps();

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

    auto& renderer = Renderer::instance();

    for (auto l : *lights)
    {
        renderer.draw(
                    std::make_shared<SphereDrawable>(2, utils::BoundingSphere(glm::vec3(), 10.0f), glm::vec4(l->color(),1)),
                    utils::Transform(glm::vec3(1,1,1), glm::quat(1,0,0,0), l->position()));

        renderer.draw(std::make_shared<FrustumDrawable>(utils::Frustum(l->m().matrix(ScenePrivate::ShadowMapZNear, ScenePrivate::ShadowMapZFar)), glm::vec4(l->color(),1)), utils::Transform());
    }

    renderer.setViewport(cameraPrivate.viewport);
    renderer.setViewMatrix(cameraPrivate.viewMatrix());
    renderer.setProjectionMatrix(cameraPrivate.projectionMatrix());
    renderer.setClearColor(cameraPrivate.clearColorBuffer, cameraPrivate.clearColor);
    renderer.setClearDepth(cameraPrivate.clearDepthBuffer, cameraPrivate.clearDepth);
    renderer.setIBLMaps(renderer.loadTexture("textures/diffuse/diffuse.json"), renderer.loadTexture("textures/specular/specular.json"));
    renderer.setLightsBuffer(getLightParamsBuffer());
    renderer.setShadowMaps(shadowMaps);

    renderer.render(nullptr);

    auto& funcs = renderer.functions();
    funcs.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer.defaultFbo());
    funcs.glBindFramebuffer(GL_READ_FRAMEBUFFER, lights->at(0)->m().shadowMapFramebuffer->id);
    funcs.glBlitFramebuffer(0,0,512,512,0,0,512,512, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

PickData ScenePrivate::pickScene(int32_t xi, int32_t yi, const CameraPrivate& cameraPrivate)
{
    static uint32_t backgroundId = 0xFFFFFFFF;

    auto& renderer = Renderer::instance();
    renderer.setViewport(cameraPrivate.viewport);
    renderer.setViewMatrix(cameraPrivate.projectionMatrix());
    renderer.setViewMatrix(cameraPrivate.viewMatrix());
    renderer.setClearColor(true, SelectionDrawable::idToColor(backgroundId));
    renderer.setClearDepth(true, 1.0f);
    renderer.setLightsBuffer(nullptr);
    renderer.setShadowMaps(nullptr);

    const float x = static_cast<float>(xi - cameraPrivate.viewport.x) / static_cast<float>(cameraPrivate.viewport.z) * 2.0f - 1.0f;
    const float y = (1.0f - static_cast<float>(yi - cameraPrivate.viewport.y) / static_cast<float>(cameraPrivate.viewport.w)) * 2.0f - 1.0f;

    auto viewProjectionMatrixInverse = glm::inverse(cameraPrivate.projectionMatrix() * cameraPrivate.viewMatrix());

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

        if (!ray.intersect(node->globalTransform() * node->boundingSphere()))
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

} // namespace
} // namespace
