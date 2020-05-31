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
    , lightIndices(std::make_shared<LightIndicesList>())
    , intersectionMode(IntersectionMode::UseBoundingBox)
    , isLightIndicesDirty(true)
    , isLocalBoundingBoxDirty(true)

{
}

void DrawableNodePrivate::addDrawable(std::shared_ptr<Drawable> drawable)
{
    drawables.insert(drawable);
    isLocalBoundingBoxDirty = true;

    dirtyBoundingBox();
    doDirtyLightIndices();
    doDirtyShadowMaps();
}

void DrawableNodePrivate::removeDrawable(std::shared_ptr<Drawable> drawable)
{
    drawables.erase(drawable);
    isLocalBoundingBoxDirty = true;

    dirtyBoundingBox();
    doDirtyLightIndices();
    doDirtyShadowMaps();
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
            if (light)
            {
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
        }

        isLightIndicesDirty = false;
    }
}

void DrawableNodePrivate::doUpdateShadowMaps()
{
    auto& renderer = Renderer::instance();

    for (auto& drawable : drawables)
        if (auto shadowDrawable = drawable->shadowDrawable())
            renderer.draw(shadowDrawable, getGlobalTransform());
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
    { // add check for including bounding box of this node to light's frustum
        if (index != -1)
            scenePrivate.dirtyShadowMap(lightsList->at(static_cast<size_t>(index)).get());
    }
}

void DrawableNodePrivate::doUpdate(uint64_t dt, uint64_t time)
{
    NodePrivate::doUpdate(dt, time);
    doUpdateLightIndices();

    auto& renderer = Renderer::instance();
    for (auto& drawable : drawables)
        renderer.draw(drawable, getGlobalTransform());

    //renderer.draw(std::make_shared<BoxDrawable>(getLocalBoundingBox(), glm::vec4(.0f, .8f, .0f, 1.0f)), getGlobalTransform());
}

void DrawableNodePrivate::doPick(uint32_t id)
{
    NodePrivate::doPick(id);

    auto& renderer = Renderer::instance();

    for (auto& drawable : drawables)
        if (auto selectionDrawable = drawable->selectionDrawable(id))
            renderer.draw(selectionDrawable, getGlobalTransform());
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
