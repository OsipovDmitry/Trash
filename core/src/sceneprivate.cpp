#include <algorithm>
#include <queue>

#include <utils/frustum.h>
#include <utils/ray.h>

#include <core/scene.h>
#include <core/light.h>
#include <core/drawablenode.h>
#include <core/scenerootnode.h>
#include <core/settings.h>
#include <core/nodevisitor.h>

#include "renderer.h"
#include "drawables.h"
#include "drawablenodeprivate.h"
#include "sceneprivate.h"
#include "scenerootnodeprivate.h"
#include "lightprivate.h"
#include "nodeupdatevisitor.h"
#include "noderendershadowmapvisitor.h"
#include "noderendervisitor.h"
#include "nodepickvisitor.h"

namespace trash
{
namespace core
{

ScenePrivate::ScenePrivate(Scene *scene)
    : thisScene(*scene)
    , rootNode(std::make_shared<SceneRootNode>(scene))
    , lights(std::make_shared<LightsList>())
    , lightsFramebuffer(std::make_shared<Framebuffer>())
    , viewMatrix(1.0f)
    , fov(glm::half_pi<float>())
    , isPerspectiveProjection(true)
{
    auto& settings = Settings::instance();
    auto& renderer = Renderer::instance();

    cameraMinZNear = settings.readFloat("Renderer.Camera.MinZNear", 1.0f);
    cameraMaxZFar = settings.readFloat("Renderer.Camera.MaxZFar", std::numeric_limits<float>::max());
    shadowMapMinZNear = settings.readFloat("Renderer.Shadow.MinZNear", 1.0f);
    shadowMapMaxZFar = settings.readFloat("Renderer.Shadow.MaxZFar", std::numeric_limits<float>::max());
    shadowMapSize = settings.readInt32("Renderer.Shadow.ShadowMapSize", 512);
    useDeferredTechnique = settings.readBool("Renderer.DeferredTechnique", false);

    renderNodesAABBs = settings.readBool("Renderer.Debug.NodesAABBs.State", false);
    nodesAABBsColor = glm::vec4(settings.readVec3("Renderer.Debug.NodesAABBs.Color"), 1.f);
    renderGeometryNodesAABBs = settings.readBool("Renderer.Debug.GeometryNodesAABBs.State", false);
    geometryNodesAABBsColor = glm::vec4(settings.readVec3("Renderer.Debug.GeometryNodesAABBs.Color"), 1.f);

    iblDiffuseMap = renderer.loadTexture(settings.readString("Renderer.IBL.DiffuseMap"));
    iblSpecularMap = renderer.loadTexture(settings.readString("Renderer.IBL.SpecularMap"));
    iblBrdfLutMap = renderer.loadTexture(settings.readString("Renderer.IBL.BrdfLutMap"));
    iblContribution = settings.readFloat("Renderer.IBL.Contribution", 0.2f);

    lightsFramebuffer->drawBuffers({GL_NONE});

    for (size_t i = 0; i < lightsDrawables.size(); ++i)
        lightsDrawables.at(i) = std::make_shared<LightDrawable>(castToLightType(i));

    iblDrawable = std::make_shared<IBLDrawable>();
}

void ScenePrivate::attachLight(std::shared_ptr<Light> light)
{
    auto& lightPrivate = light->m();
    if (lightPrivate.scene)
        lightPrivate.scene->detachLight(light);

    if (freeLightIndices.empty())
    {
        const auto numLights = lights->size();
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
        lightsShadowMaps = renderer.createTexture2DArray(GL_DEPTH_COMPONENT16, shadowMapSize, shadowMapSize, static_cast<GLint>(lights->size()), GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        lightsShadowMaps->setCompareMode(GL_COMPARE_REF_TO_TEXTURE);
        lightsShadowMaps->setCompareFunc(GL_LEQUAL);
    }

    auto freeIndexIt = freeLightIndices.begin();
    auto index = *freeIndexIt;
    freeLightIndices.erase(freeIndexIt);
    lights->at(static_cast<size_t>(index)) = light;
    dirtyLights.insert(index);
    dirtyShadowMaps.insert(index);
    lightPrivate.scene = &thisScene;
    lightPrivate.indexInScene = index;

    ScenePrivate::dirtyNodeLightIndices(*rootNode);
}

bool ScenePrivate::detachLight(std::shared_ptr<Light> light)
{
    auto& lightPrivate = light->m();

    if (&thisScene != lightPrivate.scene)
        return false;

    freeLightIndices.insert(lightPrivate.indexInScene);
    lights->at(lightPrivate.indexInScene) = nullptr;

    lightPrivate.scene = nullptr;
    lightPrivate.indexInScene = static_cast<uint32_t>(-1);

    ScenePrivate::dirtyNodeLightIndices(*rootNode);
    return true;
}

void ScenePrivate::dirtyLightParams(Light *light)
{
    dirtyLights.insert(light->m().indexInScene);
}

void ScenePrivate::dirtyShadowMap(Light *light)
{
    dirtyShadowMaps.insert(light->m().indexInScene);
}

glm::mat4x4 ScenePrivate::calcProjectionMatrix(float aspect, float zNear, float zFar)
{
    return isPerspectiveProjection ?
                glm::perspective(fov, aspect, zNear, zFar) :
                glm::ortho(-aspect * fov, +aspect * fov, -fov, +fov, zNear, zFar);
}

void ScenePrivate::dirtyNodeLightIndices(Node& dirtyNode)
{
    static NodeSimpleVisitor nv([](std::shared_ptr<Node> node){
        if (auto drawableNode = std::dynamic_pointer_cast<DrawableNode>(node))
            drawableNode->m().doDirtyLightIndices();
    });

    dirtyNode.accept(nv);
}

void ScenePrivate::dirtyNodeShadowMaps(Node& dirtyNode)
{
    static NodeSimpleVisitor nv([](std::shared_ptr<Node> node){
        if (auto drawableNode = std::dynamic_pointer_cast<DrawableNode>(node))
            drawableNode->m().doDirtyShadowMaps();
    });

    dirtyNode.accept(nv);
}

utils::Transform ScenePrivate::calcLightViewTransform(std::shared_ptr<Light> light)
{
    const glm::quat rot(light->direction(), glm::vec3(0.f, 0.f, -1.f));

    return utils::Transform(
                glm::vec3(1.f, 1.f, 1.f),
                rot,
                glm::vec3(rot * glm::vec4(-light->position(), 1.f))
                );
}

glm::mat4x4 ScenePrivate::calcLightProjMatrix(std::shared_ptr<Light> light, const std::pair<float, float>& zDistances)
{
    glm::mat4x4 result(1.f);

    switch (light->type())
    {
    case LightType::Point:
    case LightType::Spot:
    {
        result = glm::perspective(light->spotAngles().y, 1.0f, zDistances.first, zDistances.second);
        break;
    }
    case LightType::Direction:
    {
        const float halfSize = 0.5f * light->spotAngles().y;
        result = glm::ortho(-halfSize, halfSize, -halfSize, halfSize, zDistances.first, zDistances.second);
        break;
    }
    }

    return result;
}

void ScenePrivate::renderScene(uint64_t time, uint64_t dt)
{
    static const glm::mat4x4 shadowMapBiasMatrix = glm::translate(glm::mat4x4(1.f), glm::vec3(.5f)) * glm::scale(glm::mat4x4(1.f), glm::vec3(.5f));

    auto sceneBoundingBox = rootNode->globalTransform() * rootNode->boundingBox();
    auto sceneBoundingBoxCenter = sceneBoundingBox.center();
    auto sceneBoundingBoxScaledHalfSize = sceneBoundingBox.halfSize() * 1.15f;
    sceneBoundingBox = utils::BoundingBox(sceneBoundingBoxCenter - sceneBoundingBoxScaledHalfSize, sceneBoundingBoxCenter + sceneBoundingBoxScaledHalfSize);

    auto& renderer = Renderer::instance();
    float aspectRatio = static_cast<float>(renderer.viewportSize().x) / static_cast<float>(renderer.viewportSize().y);

    //updating camera
    auto distsToSceneBox = std::make_pair(cameraMinZNear, cameraMinZNear + 1.f);
    const utils::OpenFrustum cameraOpenFrustum(calcProjectionMatrix(aspectRatio, 0.0f, 1.0f) * viewMatrix);

    if (cameraOpenFrustum.contain(sceneBoundingBox))
    {
        distsToSceneBox = sceneBoundingBox.pairDistancesToPlane(cameraOpenFrustum.planes.at(4));
        if (distsToSceneBox.second > cameraMinZNear)
            distsToSceneBox = { glm::max(cameraMinZNear, distsToSceneBox.first),
                                glm::min(cameraMaxZFar, distsToSceneBox.second) };
    }

    const glm::mat4x4 projectionMatrix = calcProjectionMatrix(aspectRatio, distsToSceneBox.first, distsToSceneBox.second);
    const utils::Frustum cameraFrustum(projectionMatrix * viewMatrix);

    // updating nodes
    NodeUpdateVisitor nodeUpdateVisitor(time, dt);
    rootNode->accept(nodeUpdateVisitor);

    // updating lights and shadows
    for (auto lightIdx : dirtyLights)
    {
        auto light = lights->at(lightIdx);
        if (light)
        {
            auto *p = reinterpret_cast<glm::mat4x4*>(lightsUbo->map(sizeof(glm::mat4x4) * (2 * lightIdx + 0), sizeof(glm::mat4x4), GL_MAP_WRITE_BIT));
            *p = light->m().packParams();
            lightsUbo->unmap();
        }
    }
    dirtyLights.clear();

    for (auto it = dirtyShadowMaps.begin(); it != dirtyShadowMaps.end(); )
    {
        const auto lightIdx = *it;
        auto light = lights->at(lightIdx);
        if (!light)
        {
            it = dirtyShadowMaps.erase(it);
            continue;
        }

        if (!light->isShadowMapEnabled())
        {
            it = dirtyShadowMaps.erase(it);
            continue;
        }

        const utils::OpenFrustum lightOpenFrustum(calcLightProjMatrix(light, {0.0f, 1.0f}) * calcLightViewTransform(light));
        auto dists = std::make_pair(shadowMapMinZNear, shadowMapMinZNear + 1.0f);

        if (!lightOpenFrustum.contain(sceneBoundingBox))
        {
            it = dirtyShadowMaps.erase(it);
            continue;
        }

        dists = sceneBoundingBox.pairDistancesToPlane(lightOpenFrustum.planes.at(4));
        if (dists.second < shadowMapMinZNear)
        {
            it = dirtyShadowMaps.erase(it);
            continue;
        }
        dists = { glm::max(shadowMapMinZNear, dists.first), glm::min(shadowMapMaxZFar, dists.second) }; // add dependency on light's radius
        const auto lightMatrix = calcLightProjMatrix(light, dists) * calcLightViewTransform(light);

        const utils::Frustum lightFrustum(lightMatrix);
        if (!cameraFrustum.contain(lightFrustum))
        {
            ++it;
            continue;
        }

        NodeRenderShadowMapVisitor nodeRenderShadowMapVisitor(lightFrustum);
        rootNode->accept(nodeRenderShadowMapVisitor);

        lightsFramebuffer->attachDepth(lightsShadowMaps, 0u, lightIdx);
        renderer.renderShadows(RenderInfo(glm::mat4x4(1.0f), lightMatrix), lightsFramebuffer, glm::uvec2(shadowMapSize, shadowMapSize));
        renderer.clear();

        auto *p = reinterpret_cast<glm::mat4x4*>(lightsUbo->map(sizeof(glm::mat4x4) * (2 * lightIdx + 1), sizeof(glm::mat4x4), GL_MAP_WRITE_BIT));
        *p = shadowMapBiasMatrix * lightMatrix;
        lightsUbo->unmap();

        it = dirtyShadowMaps.erase(it);
    }

    // render nodes
    NodeRenderVisitor nodeRenderVisitor(cameraFrustum);
    rootNode->accept(nodeRenderVisitor);

    if (useDeferredTechnique)
    {
        renderer.draw(iblDrawable, utils::Transform(sceneBoundingBox.halfSize(), glm::quat(1.f, 0.f, 0.f, 0.f), sceneBoundingBox.center()), static_cast<uint32_t>(0));

        for (size_t lightIdx = 0; lightIdx < lights->size(); ++lightIdx)
        {
            auto light = lights->at(lightIdx);

            if (!light)
                continue;

            utils::Transform lightViewTransformInverse;

            switch (light->type()) {
            case LightType::Point:
            {
                const auto& pos = light->position();
                const auto& rads = light->radiuses();
                const float radius = rads.x + rads.y;

                if (!cameraFrustum.contain(utils::BoundingSphere(pos, radius)))
                    continue;

                lightViewTransformInverse = calcLightViewTransform(light).inverted();
                const float scaledRadius = radius * 1.15f;

                lightViewTransformInverse.scale *= glm::vec3(scaledRadius, scaledRadius, scaledRadius);
                break;
            }
            case LightType::Spot:
            {
                const auto& pos = light->position();
                const auto& rads = light->radiuses();
                const float radius = rads.x + rads.y;

                if (!cameraFrustum.contain(utils::BoundingSphere(pos, radius)))
                    continue;

                lightViewTransformInverse = calcLightViewTransform(light).inverted();
                const float scaledRadius = radius * 1.15f;

                const float tan = glm::tan(light->spotAngles().y * .5f);
                lightViewTransformInverse.scale *= glm::vec3(scaledRadius*tan, scaledRadius*tan, scaledRadius);
                break;
            }
            case LightType::Direction:
            {
                if (!cameraFrustum.contain(sceneBoundingBox))
                    continue;

                lightViewTransformInverse = utils::Transform(sceneBoundingBox.halfSize(), glm::quat(1.f, 0.f, 0.f, 0.f), sceneBoundingBox.center());
                break;
            }
            }

            renderer.draw(lightsDrawables[castFromLightType(light->type())], lightViewTransformInverse, static_cast<uint32_t>(lightIdx));
        }
    }

    RenderInfo renderInfo(viewMatrix, projectionMatrix);
    renderInfo.setIBLData(iblDiffuseMap, iblSpecularMap, iblBrdfLutMap, iblContribution);
    renderInfo.setLightsBuffer(lightsUbo);
    renderInfo.setShadowMaps(lightsShadowMaps);

    useDeferredTechnique ?
                renderer.renderDeffered(renderInfo) :
                renderer.renderForward(renderInfo);
    renderer.clear();
}

PickData ScenePrivate::pickScene(int32_t xi, int32_t yi)
{
    auto& renderer = Renderer::instance();
    const auto& viewportSize = renderer.viewportSize();
    float aspectRatio = static_cast<float>(renderer.viewportSize().x) / static_cast<float>(renderer.viewportSize().y);

    auto sceneBoundingBox = rootNode->globalTransform() * rootNode->boundingBox();
    auto sceneBoundingBoxCenter = sceneBoundingBox.center();
    auto sceneBoundingBoxScaledHalfSize = sceneBoundingBox.halfSize() * 1.15f;
    sceneBoundingBox = utils::BoundingBox(sceneBoundingBoxCenter - sceneBoundingBoxScaledHalfSize, sceneBoundingBoxCenter + sceneBoundingBoxScaledHalfSize);

    auto distsToSceneBox = std::make_pair(cameraMinZNear, cameraMinZNear + 1.f);
    const utils::OpenFrustum cameraOpenFrustum(calcProjectionMatrix(aspectRatio, 0.0f, 1.0f) * viewMatrix);

    if (cameraOpenFrustum.contain(sceneBoundingBox))
    {
        distsToSceneBox = sceneBoundingBox.pairDistancesToPlane(cameraOpenFrustum.planes.at(4));
        if (distsToSceneBox.second > cameraMinZNear)
            distsToSceneBox = { glm::max(cameraMinZNear, distsToSceneBox.first),
                                glm::min(cameraMaxZFar, distsToSceneBox.second) };
    }

    const glm::mat4x4 projectionMatrix = calcProjectionMatrix(aspectRatio, distsToSceneBox.first, distsToSceneBox.second);
    RenderInfo renderInfo(viewMatrix, projectionMatrix);

    const float x = static_cast<float>(xi) / static_cast<float>(viewportSize.x) * 2.0f - 1.0f;
    const float y = (1.0f - static_cast<float>(yi) / static_cast<float>(viewportSize.y)) * 2.0f - 1.0f;

    glm::vec4 p0 = renderInfo.viewProjMatrixInverse() * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 p1 = renderInfo.viewProjMatrixInverse() * glm::vec4(x, y, 1.0f, 1.0f);

    p0 /= p0.w;
    p1 /= p1.w;

    NodePickVisitor nodePickVisitor(utils::Ray(p0, p1-p0));
    rootNode->accept(nodePickVisitor);

    auto pickBuffer = std::make_shared<Framebuffer>();
    pickBuffer->attachColor(0, std::make_shared<Renderbuffer>(GL_R32UI, viewportSize.x, viewportSize.y));
    pickBuffer->attachDepth(std::make_shared<Renderbuffer>(GL_DEPTH_COMPONENT32, viewportSize.x, viewportSize.y));

    renderer.renderIds(renderInfo, pickBuffer, viewportSize);
    renderer.clear();

    GLuint id[1] = {0u};
    GLfloat depth[1] = {1.0f};

    renderer.readPixel(pickBuffer, GL_COLOR_ATTACHMENT0, xi, viewportSize.y - yi - 1, GL_RED_INTEGER, GL_UNSIGNED_INT, static_cast<GLvoid*>(id));
    renderer.readPixel(pickBuffer, GL_DEPTH_ATTACHMENT, xi, viewportSize.y - yi - 1, GL_DEPTH_COMPONENT, GL_FLOAT, static_cast<GLvoid*>(depth));

    std::shared_ptr<DrawableNode> node = (id[0] > 0 && id[0] <= nodePickVisitor.nodeIds().size()) ? nodePickVisitor.nodeIds()[id[0] - 1] : nullptr;

    depth[0] = depth[0] * 2.0f - 1.0f;
    p0 = renderInfo.viewProjMatrixInverse() * glm::vec4(x, y, depth[0], 1.0f);
    p0 /= p0.w;

    glm::vec3 localCoord(0.0f, 0.0f, 0.0f);
    if (node)
        localCoord = node->globalTransform().inverted() * glm::vec3(p0.x, p0.y, p0.z);

    return PickData{node, localCoord};
}

utils::Ray ScenePrivate::throwRay(int32_t xi, int32_t yi)
{
    auto& renderer = Renderer::instance();
    const auto& viewportSize = renderer.viewportSize();
    float aspectRatio = static_cast<float>(renderer.viewportSize().x) / static_cast<float>(renderer.viewportSize().y);

    auto sceneBoundingBox = rootNode->globalTransform() * rootNode->boundingBox();
    auto sceneBoundingBoxCenter = sceneBoundingBox.center();
    auto sceneBoundingBoxScaledHalfSize = sceneBoundingBox.halfSize() * 1.15f;
    sceneBoundingBox = utils::BoundingBox(sceneBoundingBoxCenter - sceneBoundingBoxScaledHalfSize, sceneBoundingBoxCenter + sceneBoundingBoxScaledHalfSize);

    auto distsToSceneBox = std::make_pair(cameraMinZNear, cameraMinZNear + 1.f);
    const utils::OpenFrustum cameraOpenFrustum(calcProjectionMatrix(aspectRatio, 0.0f, 1.0f) * viewMatrix);

    if (cameraOpenFrustum.contain(sceneBoundingBox))
    {
        distsToSceneBox = sceneBoundingBox.pairDistancesToPlane(cameraOpenFrustum.planes.at(4));
        if (distsToSceneBox.second > cameraMinZNear)
            distsToSceneBox = { glm::max(cameraMinZNear, distsToSceneBox.first),
                                glm::min(cameraMaxZFar, distsToSceneBox.second) };
    }

    const glm::mat4x4 projectionMatrix = calcProjectionMatrix(aspectRatio, distsToSceneBox.first, distsToSceneBox.second);

    const float xf = static_cast<float>(xi) / static_cast<float>(viewportSize.x) * 2.0f - 1.0f;
    const float yf = (1.0f - static_cast<float>(yi) / static_cast<float>(viewportSize.y)) * 2.0f - 1.0f;

    auto viewProjectionMatrixInverse = glm::inverse(projectionMatrix * viewMatrix);
    glm::vec4 p0 = viewProjectionMatrixInverse * glm::vec4(xf, yf, -1.0f, 1.0f);
    glm::vec4 p1 = viewProjectionMatrixInverse * glm::vec4(xf, yf, 1.0f, 1.0f);

    p0 /= p0.w;
    p1 /= p1.w;

    return utils::Ray(p0, p1-p0);
}

} // namespace
} // namespace
