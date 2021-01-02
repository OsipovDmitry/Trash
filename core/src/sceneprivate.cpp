#include <algorithm>
#include <queue>

#include <utils/frustum.h>
#include <utils/ray.h>

#include <core/scene.h>
#include <core/light.h>
#include <core/camera.h>
#include <core/drawablenode.h>
#include <core/scenerootnode.h>
#include <core/settings.h>

#include "renderer.h"
#include "drawables.h"
#include "cameraprivate.h"
#include "drawablenodeprivate.h"
#include "sceneprivate.h"
#include "scenerootnodeprivate.h"
#include "lightprivate.h"

#include <QOpenGLExtraFunctions>

namespace trash
{
namespace core
{

ScenePrivate::ScenePrivate(Scene *scene)
    : thisScene(*scene)
    , rootNode(std::make_shared<SceneRootNode>(scene))
    , camera(std::make_shared<Camera>())
    , lights(std::make_shared<LightsList>())
    , lightsFramebuffer(std::make_shared<Framebuffer>())
    , postEffectDrawable(std::make_shared<PostEffectDrawable>())
{
    auto& settings = Settings::instance();
    auto& renderer = Renderer::instance();

    cameraMinZNear = settings.readFloat("Renderer.Camera.MinZNear", 1.0f);
    cameraMaxZFar = settings.readFloat("Renderer.Camera.MaxZFar", std::numeric_limits<float>::max());
    shadowMapMinZNear = settings.readFloat("Renderer.Shadow.MinZNear", 1.0f);
    shadowMapMaxZFar = settings.readFloat("Renderer.Shadow.MaxZFar", std::numeric_limits<float>::max());
    shadowMapSize = settings.readInt32("Renderer.Shadow.ShadowMapSize", 512);
    useDeferredTechnique = settings.readBool("Renderer.DeferredTechnique", false);

    iblDiffuseMap = renderer.loadTexture(settings.readString("Renderer.IBL.DiffuseMap"));
    iblSpecularMap = renderer.loadTexture(settings.readString("Renderer.IBL.SpecularMap"));
    iblBrdfLutMap = renderer.loadTexture(settings.readString("Renderer.IBL.BrdfLutMap"));
    iblContribution = settings.readFloat("Renderer.IBL.Contribution", 0.2f);

    backgroundDrawable = std::make_shared<BackgroundDrawable>(settings.readFloat("Renderer.Background.Roughness", 0.05f));

    //lightsFramebuffer->attachColor(0, std::make_shared<Renderbuffer>(GL_RGBA8, ShadowMapSize, ShadowMapSize));
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
        lightsShadowMaps->setWrap(GL_CLAMP_TO_BORDER);
        lightsShadowMaps->setBorderColor(glm::vec4(1.f, 1.f, 1.f, 1.f));
        lightsShadowMaps->setCompareMode(GL_COMPARE_REF_TO_TEXTURE);
        lightsShadowMaps->setCompareFunc(GL_LEQUAL);

        attachLight(light);
    }

    ScenePrivate::dirtyNodeLightIndices(*rootNode);
}

bool ScenePrivate::detachLight(std::shared_ptr<Light> light)
{
    auto& lightPrivate = light->m();

    if (&thisScene != lightPrivate.scene)
        return false;

    auto lightIter = std::find(lights->begin(), lights->end(), light);
    if (lightIter == lights->end())
        return false;

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

void ScenePrivate::dirtyShadowMap(Light *light)
{
    dirtyShadowMaps.insert(static_cast<uint32_t>(std::distance(
                                                     lights->begin(),
                                                     std::find_if(lights->begin(), lights->end(),
                                                                  [light](const std::shared_ptr<trash::core::Light>& l){ return l.get() == light; }))
                                                 ));
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
    auto sceneBoundingBoxScaledHalsSize = sceneBoundingBox.halfSize() * 1.15f;
    sceneBoundingBox = utils::BoundingBox(sceneBoundingBoxCenter - sceneBoundingBoxScaledHalsSize, sceneBoundingBoxCenter + sceneBoundingBoxScaledHalsSize);

    auto& renderer = Renderer::instance();

    //update camera
    auto distsToSceneBox = std::make_pair(cameraMinZNear, cameraMinZNear + 1.f);
    auto& cameraPrivate = camera->m();
    const utils::OpenFrustum cameraOpenFrustum(cameraPrivate.calcProjectionMatrix({0.0f, 1.0f}) * camera->viewMatrix());

    if (cameraOpenFrustum.contain(sceneBoundingBox))
    {
        distsToSceneBox = sceneBoundingBox.pairDistancesToPlane(cameraOpenFrustum.planes.at(4));
        if (distsToSceneBox.second > cameraMinZNear)
            distsToSceneBox = { glm::max(cameraMinZNear, distsToSceneBox.first),
                                glm::min(cameraMaxZFar, distsToSceneBox.second) };
    }

    cameraPrivate.setZPlanes(distsToSceneBox);
    const utils::Frustum cameraFrustum(camera->projectionMatrix() * camera->viewMatrix());

    // updating nodes
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
        dists = { glm::max(shadowMapMinZNear, dists.first), glm::min(shadowMapMaxZFar, dists.second) };
        const auto lightMatrix = calcLightProjMatrix(light, dists) * calcLightViewTransform(light);

        const utils::Frustum lightFrustum(lightMatrix);
        if (!cameraFrustum.contain(lightFrustum))
        {
            ++it;
            continue;
        }

        nodes.push(rootNode);
        while (!nodes.empty())
        {
            auto node = nodes.front();
            nodes.pop();

            if (!lightFrustum.contain(node->globalTransform() * node->boundingBox()))
                continue;

            if (auto drawableNode = node->asDrawableNode())
            {
                if (lightFrustum.contain(drawableNode->globalTransform() * drawableNode->m().getLocalBoundingBox()))
                    drawableNode->m().doRender(0);
            }

            for (auto child : node->children())
                nodes.push(child);
        }

        lightsFramebuffer->attachDepth(lightsShadowMaps, static_cast<uint32_t>(lightIdx));
        renderer.renderShadows(RenderInfo(glm::mat4x4(1.0f), lightMatrix, glm::uvec2(shadowMapSize, shadowMapSize)), lightsFramebuffer);
        renderer.clear();

        auto *p = reinterpret_cast<glm::mat4x4*>(lightsUbo->map(sizeof(glm::mat4x4) * (2 * lightIdx + 1), sizeof(glm::mat4x4), GL_MAP_WRITE_BIT));
        *p = shadowMapBiasMatrix * lightMatrix;
        lightsUbo->unmap();

        it = dirtyShadowMaps.erase(it);
    }

    // render nodes
    nodes.push(rootNode);
    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        if (!cameraFrustum.contain(node->globalTransform() * node->boundingBox()))
            continue;

        if (auto drawableNode = node->asDrawableNode())
        {
            if (cameraFrustum.contain(drawableNode->globalTransform() * drawableNode->m().getLocalBoundingBox()))
                drawableNode->m().doRender(0);
        }

        for (auto child : node->children())
            nodes.push(child);
    }

//    for (auto light: *lights)
//    {
//        if (!light)
//            continue;

//        auto lightViewTransformInverse = calcLightViewTransform(light).inverse();
//        const float scaledRadius = (light->radiuses().x + light->radiuses().y) * 1.15f;

//        if (light->type() == LightType::Point)
//        {
//            lightViewTransformInverse.scale *= glm::vec3(scaledRadius, scaledRadius, scaledRadius);
//            renderer.draw(std::make_shared<SphereDrawable>(6, utils::BoundingSphere(glm::vec3(), 1.0f), glm::vec4(light->color(), 1)),
//                          lightViewTransformInverse,
//                          0);
//        }
//        else if (light->type() == LightType::Spot)
//        {
//            const float tan = glm::tan(light->spotAngles().y * .5f);
//            lightViewTransformInverse.scale *= glm::vec3(scaledRadius*tan, scaledRadius*tan, scaledRadius);
//            renderer.draw(std::make_shared<ConeDrawable>(6, 1.f, 1.f, glm::vec4(light->color(), 1)),
//                          lightViewTransformInverse,
//                          0);
//        }
//    }

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

                lightViewTransformInverse = calcLightViewTransform(light).inverse();
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

                lightViewTransformInverse = calcLightViewTransform(light).inverse();
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

//            renderer.draw(std::make_shared<SphereDrawable>(2, utils::BoundingSphere(glm::vec3(), 10.0f), glm::vec4(light->color(),1)),
//                        utils::Transform(glm::vec3(1,1,1), glm::quat(1,0,0,0), light->position()), 0);

//            const utils::OpenFrustum lightOpenFrustum(calcLightProjMatrix(light, {0.0f, 1.0f}) * calcLightViewTransform(light));
//            auto dists = std::make_pair(ShadowMapMinZNear, ShadowMapMinZNear + 1.0f);
//            dists = sceneBoundingBox.pairDistancesToPlane(lightOpenFrustum.planes.at(4));
//            dists = { glm::max(ShadowMapMinZNear, dists.first), glm::min(ShadowMapMaxZFar, dists.second) };
//            const auto lightMatrix = calcLightProjMatrix(light, dists) * calcLightViewTransform(light);
//            renderer.draw(std::make_shared<FrustumDrawable>(utils::Frustum(lightMatrix), glm::vec4(light->color(),1)), utils::Transform(), 0);
        }
    }

    RenderInfo renderInfo(camera->viewMatrix(), camera->projectionMatrix(), camera->viewportSize());
    renderInfo.setIBLData(iblDiffuseMap, iblSpecularMap, iblBrdfLutMap, iblContribution);
    renderInfo.setLightsBuffer(lightsUbo);
    renderInfo.setShadowMaps(lightsShadowMaps);

    renderer.draw(backgroundDrawable, utils::Transform(), 0);
    renderer.draw(postEffectDrawable, utils::Transform(), 0);

    useDeferredTechnique ?
                renderer.renderDeffered(renderInfo) :
                renderer.renderForward(renderInfo);
    renderer.clear();
}

PickData ScenePrivate::pickScene(int32_t xi, int32_t yi)
{
    auto& renderer = Renderer::instance();
    const auto& viewportSize = camera->viewportSize();

    const float x = static_cast<float>(xi) / static_cast<float>(viewportSize.x) * 2.0f - 1.0f;
    const float y = (1.0f - static_cast<float>(yi) / static_cast<float>(viewportSize.y)) * 2.0f - 1.0f;

    auto viewProjectionMatrixInverse = glm::inverse(camera->projectionMatrix() * camera->viewMatrix());

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

        if (auto drawableNode = node->asDrawableNode())
        {
            nodeIds.push_back(node);
            drawableNode->m().doRender(static_cast<uint32_t>(nodeIds.size()));
        }

        for (auto child : node->children())
            nodes.push(child);
    }

    auto pickBuffer = std::make_shared<Framebuffer>();
    pickBuffer->attachColor(0, std::make_shared<Renderbuffer>(GL_R32UI, viewportSize.x, viewportSize.y));
    pickBuffer->attachDepth(std::make_shared<Renderbuffer>(GL_DEPTH_COMPONENT32, viewportSize.x, viewportSize.y));

    RenderInfo renderInfo(camera->viewMatrix(), camera->projectionMatrix(), viewportSize);

    renderer.renderIds(renderInfo, pickBuffer);
    renderer.clear();

    GLuint id[1] = {0u};
    GLfloat depth[1] = {1.0f};

    renderer.readPixel(renderInfo, pickBuffer, GL_COLOR_ATTACHMENT0, xi, yi, GL_RED_INTEGER, GL_UNSIGNED_INT, static_cast<GLvoid*>(id));
    renderer.readPixel(renderInfo, pickBuffer, GL_DEPTH_ATTACHMENT, xi, yi, GL_DEPTH_COMPONENT, GL_FLOAT, static_cast<GLvoid*>(depth));

    std::shared_ptr<Node> node = (id[0] > 0 && id[0] <= nodeIds.size()) ? nodeIds[id[0] - 1] : nullptr;

    depth[0] = depth[0] * 2.0f - 1.0f;
    p0 = viewProjectionMatrixInverse * glm::vec4(x, y, depth[0], 1.0f);
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
                std::pair<float, float> distsToBox = box.pairDistancesToPlane(frustum.planes.at(4));
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

} // namespace
} // namespace
