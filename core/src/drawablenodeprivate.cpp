#include <core/scene.h>
#include <core/light.h>

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
    , lightIndices(std::make_shared<LightIndicesList>())
    , isLightIndicesDirty(true)
    , isLocalBoundingSphereDirty(true)

{

}

utils::BoundingSphere DrawableNodePrivate::getLocalBoundingSphere()
{
    if (isLocalBoundingSphereDirty)
    {
        localBoundingSphere = utils::BoundingSphere();
        for (auto drawable : drawables)
            localBoundingSphere += drawable->mesh()->boundingSphere;

        isLocalBoundingSphereDirty = false;
    }

    return localBoundingSphere;
}

std::shared_ptr<LightIndicesList> DrawableNodePrivate::getLightIndices()
{
    updateLightIndices();
    return lightIndices;
}

void DrawableNodePrivate::doUpdate(uint64_t, uint64_t)
{
    updateLightIndices();

    auto& renderer = Renderer::instance();
    for (auto& drawable : drawables)
        renderer.draw(drawable, getGlobalTransform());
}

void DrawableNodePrivate::doPick(uint32_t id)
{
    auto& renderer = Renderer::instance();

    for (auto& drawable : drawables)
        renderer.draw(drawable->selectionDrawable(id), getGlobalTransform());
}

void DrawableNodePrivate::doUpdateShadowMaps()
{
    auto& renderer = Renderer::instance();

    for (auto& drawable : drawables)
        renderer.draw(drawable->shadowDrawable(), getGlobalTransform());
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
    auto lightsList = scene->lights();
    auto lightIndicesList = getLightIndices();

    for (auto index : *lightIndicesList)
    { // add check for including bounding sphere of this node to light's frustum
        if (index != -1)
            scenePrivate.dirtyShadowMap(lightsList->at(static_cast<size_t>(index)).get());
    }

}

void DrawableNodePrivate::updateLightIndices()
{
    if (isLightIndicesDirty)
    {
        auto lightsList = getScene()->m().lights;

        std::array<float, MAX_LIGHTS_PER_NODE> intesities;
        for (size_t i = 0; i < MAX_LIGHTS_PER_NODE; ++i)
        {
            intesities[i] = 0.0f;
            lightIndices->at(i) = -1;
        }

        for (size_t lightIndex = 0; lightIndex < lightsList->size(); ++lightIndex)
        {
            auto light = lightsList->at(lightIndex);
            float lightIntensity = light->m().intensity(getGlobalTransform().translation);
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

        isLightIndicesDirty = false;
    }
}


} // namespace
} // namespace
