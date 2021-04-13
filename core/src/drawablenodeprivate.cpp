#include <utils/frustum.h>

#include <core/scene.h>
#include <core/light.h>
#include <core/drawablenode.h>

#include "drawablenodeprivate.h"
#include "sceneprivate.h"
#include "lightprivate.h"
#include "renderer.h"
#include "drawables.h"

namespace trash
{
namespace core
{

DrawableNodePrivate::DrawableNodePrivate(Node &node)
    : NodePrivate(node)
    , lightIndices(std::make_shared<LightIndicesList>(true))
    , intersectionMode(IntersectionMode::UseBoundingBox)
    , isLightIndicesDirty(true)
    , isLocalBoundingBoxDirty(true)
    , areShadowsEnabled(true)

{
}

void DrawableNodePrivate::addDrawable(std::shared_ptr<Drawable> drawable)
{
    drawables.insert(drawable);
    isLocalBoundingBoxDirty = true;

    dirtyBoundingBox();
    doDirtyLightIndices();
    doDirtyShadowMaps();

    drawable->dirtyCache();
}

void DrawableNodePrivate::removeDrawable(std::shared_ptr<Drawable> drawable)
{
    drawables.erase(drawable);
    isLocalBoundingBoxDirty = true;

    dirtyBoundingBox();
    doDirtyLightIndices();
    doDirtyShadowMaps();

    drawable->dirtyCache();
}

void DrawableNodePrivate::removeAllDrawables()
{
    drawables.clear();
    isLocalBoundingBoxDirty = true;

    dirtyBoundingBox();
    doDirtyLightIndices();
    doDirtyShadowMaps();
}

void DrawableNodePrivate::dirtyDrawables()
{
    for (auto drawable : drawables)
        drawable->dirtyCache();
}

const utils::BoundingBox &DrawableNodePrivate::getLocalBoundingBox()
{
    if (isLocalBoundingBoxDirty)
    {
        localBoundingBox = utils::BoundingBox();
        for (auto drawable : drawables)
            localBoundingBox += drawable->mesh()->boundingBox;

        isLocalBoundingBoxDirty = false;
    }

    return localBoundingBox;
}

std::shared_ptr<LightIndicesList> DrawableNodePrivate::getLightIndices()
{
    doUpdateLightIndices();
    return lightIndices;
}

void DrawableNodePrivate::doUpdateLightIndices()
{
    if (!getScene())
        return;

    if (isLightIndicesDirty && lightIndices->isEnabled)
    {
        auto lightsList = getScene()->m().lights;
        auto boundingBox = getGlobalTransform() * getLocalBoundingBox();

        std::array<float, MAX_LIGHTS_PER_NODE> intesities;
        for (size_t i = 0; i < MAX_LIGHTS_PER_NODE; ++i)
        {
            intesities[i] = 0.0f;
            lightIndices->at(i) = -1;
        }

        for (size_t lightIndex = 0; lightIndex < lightsList->size(); ++lightIndex)
        {
            auto light = lightsList->at(lightIndex);
            if (light)
            {
                float lightIntensity = light->m().intensity(boundingBox);
                for (int32_t i = 0; i < MAX_LIGHTS_PER_NODE; ++i)
                {
                    if (intesities[static_cast<size_t>(i)] < lightIntensity)
                    {
                        std::copy_backward(intesities.begin()+i, intesities.end()-1, intesities.end());
                        std::copy_backward(lightIndices->begin()+i, lightIndices->end()-1, lightIndices->end());

                        intesities[static_cast<size_t>(i)] = lightIntensity;
                        lightIndices->at(static_cast<size_t>(i)) = static_cast<int32_t>(lightIndex);
                        break;
                    }
                }
            }
        }
        isLightIndicesDirty = false;
    }
}

void DrawableNodePrivate::doDirtyLightIndices()
{
    isLightIndicesDirty = true;
}

void DrawableNodePrivate::doDirtyShadowMaps()
{
    auto* scene = getScene();
    if (!scene)
        return;

    auto& scenePrivate = scene->m();
    auto lightsList = scenePrivate.lights;
    auto boundingBox = getGlobalTransform() * getLocalBoundingBox();

    for (auto light : *lightsList)
    {
        if (!light)
            continue;

        const utils::OpenFrustum lightOpenFrustum(ScenePrivate::calcLightProjMatrix(light, {0.0f, 1.0f}) * ScenePrivate::calcLightViewTransform(light));
        if (lightOpenFrustum.contain(boundingBox))
            scenePrivate.dirtyShadowMap(light.get());
    }
}

void DrawableNodePrivate::doRender(uint32_t id)
{
    auto& renderer = Renderer::instance();
    for (auto& drawable : drawables)
        renderer.draw(drawable, getGlobalTransform(), id);

    //renderer.draw(std::make_shared<BoxDrawable>(getLocalBoundingBox(), glm::vec4(.0f, .8f, .0f, 1.0f)), getGlobalTransform());
}

void DrawableNodePrivate::doUpdate(uint64_t time, uint64_t dt)
{
    NodePrivate::doUpdate(time, dt);
    doUpdateLightIndices();
}

void DrawableNodePrivate::doBeforeChangingTransformation()
{
    NodePrivate::doBeforeChangingTransformation();
    ScenePrivate::dirtyNodeShadowMaps(thisNode);
}

void DrawableNodePrivate::doAfterChangingTransformation()
{
    NodePrivate::doAfterChangingTransformation();
    ScenePrivate::dirtyNodeLightIndices(thisNode);
    ScenePrivate::dirtyNodeShadowMaps(thisNode);
}

} // namespace
} // namespace
