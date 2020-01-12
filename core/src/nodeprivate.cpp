#include <core/node.h>
#include <core/light.h>
#include <core/scene.h>

#include "nodeprivate.h"
#include "sceneprivate.h"
#include "drawables.h"
#include "renderer.h"
#include "sceneprivate.h"
#include "lightprivate.h"

namespace trash
{
namespace core
{

NodePrivate::NodePrivate(Node &node)
    : thisNode(node)
    , minimalBoundingSphere()
    , isGlobalTransformDirty(true)
    , isBoundingSphereDirty(true)
    , isLightsDirty(true)
{
}

NodePrivate::~NodePrivate()
{
}

void NodePrivate::dirtyGlobalTransform()
{
    isGlobalTransformDirty = true;
    for (auto child : thisNode.children())
        child->m().dirtyGlobalTransform();
}

void NodePrivate::dirtyBoundingSphere()
{
    isBoundingSphereDirty = true;
    if (thisNode.parent())
        thisNode.parent()->m().dirtyBoundingSphere();
}

void NodePrivate::dirtyLights()
{
    isLightsDirty = true;
    for (auto child : thisNode.children())
        child->m().dirtyLights();
}

void NodePrivate::doUpdate(uint64_t, uint64_t)
{
    getLights();

    auto& renderer = Renderer::instance();
    for (auto& drawable : drawables)
        renderer.draw(drawable, getGlobalTransform());
}

void NodePrivate::doPick(uint32_t id)
{
    auto& renderer = Renderer::instance();

    for (auto& drawable : drawables)
        renderer.draw(drawable->selectionDrawable(id), thisNode.globalTransform());
}

Scene *NodePrivate::getScene() const
{
    Node *node = &thisNode;

    while (node->parent())
        node = node->parent();

    if (auto sceneRootNode = dynamic_cast<SceneRootNode*>(node))
    {
        return sceneRootNode->scene();
    }

    return nullptr;
}

const utils::BoundingSphere &NodePrivate::getBoundingSphere()
{
    if (isBoundingSphereDirty)
    {
        boundingSphere = minimalBoundingSphere;
        for (auto drawable : drawables)
            boundingSphere += drawable->mesh()->boundingSphere;
        for (auto child : thisNode.children())
            boundingSphere += child->transform() * child->boundingSphere();
        isBoundingSphereDirty = false;

//        drawables.erase(bSphereDrawable);
//        bSphereDrawable = std::make_shared<SphereDrawable>(8, boundingSphere, glm::vec4(0.8f, .0f, .0f, 1.0f));
//        drawables.insert(bSphereDrawable);
    }
    return boundingSphere;
}

const utils::Transform &NodePrivate::getGlobalTransform()
{
    if (isGlobalTransformDirty)
    {
        globalTransform = transform;

        auto parent = thisNode.parent();
        if (parent)
            globalTransform = parent->globalTransform() * globalTransform;

        isGlobalTransformDirty = false;
    }

    return globalTransform;
}

const LightIndicesList &NodePrivate::getLights()
{
    if (isLightsDirty)
    {
        auto& lightsList = getScene()->m().lights;

        std::array<float, MAX_LIGHTS_PER_NODE> intesities;
        for (size_t i = 0; i < MAX_LIGHTS_PER_NODE; ++i)
        {
            intesities[i] = 0.0f;
            lights[i] = -1;
        }

        for (size_t lightIndex = 0; lightIndex < lightsList.size(); ++lightIndex)
        {
            auto light = lightsList[lightIndex];
            float lightIntensity = light->m().intensity(getGlobalTransform().translation);
            for (size_t i = 0; i < MAX_LIGHTS_PER_NODE; ++i)
            {
                if (intesities[static_cast<size_t>(i)] < lightIntensity)
                {
                    std::copy_backward(intesities.begin()+i, intesities.end()-1, intesities.end());
                    std::copy_backward(lights.begin()+i, lights.end()-1, lights.end());

                    intesities[static_cast<size_t>(i)] = lightIntensity;
                    lights[static_cast<size_t>(i)] = static_cast<int32_t>(lightIndex);

                    break;
                }
            }
        }

        isLightsDirty = false;
    }

    return lights;
}

void NodePrivate::addDrawable(std::shared_ptr<Drawable> drawable)
{
    drawables.insert(drawable);
    dirtyBoundingSphere();
}

void NodePrivate::removeDrawable(std::shared_ptr<Drawable> drawable)
{
    drawables.erase(drawable);
    dirtyBoundingSphere();
}

} // namespace
} // namespace
