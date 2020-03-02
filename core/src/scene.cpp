#include <core/scene.h>
#include <core/light.h>

#include "sceneprivate.h"
#include "nodeprivate.h"
#include "lightprivate.h"

namespace trash
{
namespace core
{

Scene::Scene()
    : m_(std::make_unique<ScenePrivate>(this))
{
}

Scene::~Scene()
{
}

std::shared_ptr<Node> Scene::rootNode()
{
    return m_->rootNode;
}

std::shared_ptr<const Node> Scene::rootNode() const
{
    return m_->rootNode;
}

void Scene::attachLight(std::shared_ptr<Light> light)
{
    auto& lightPrivate = light->m();
    if (lightPrivate.scene)
        lightPrivate.scene->detachLight(light);

    lightPrivate.scene = this;

    if (!m_->freeLights.empty())
    {
        auto freeIndexIt = m_->freeLights.begin();
        auto index = *freeIndexIt;
        m_->freeLights.erase(freeIndexIt);
        m_->lights->at(static_cast<size_t>(index)) = light;
        m_->dirtyLights.insert(index);
        m_->dirtyShadowMaps.insert(index);

    }
    else
    {
        m_->lights->push_back(light);
        m_->allLightsAreDirty = true;
        m_->allShadowMapsAreDirty = true;
    }

    m_->rootNode->m().dirtyLightIndices();
}

bool Scene::detachLight(std::shared_ptr<Light> light)
{
    auto& lightPrivate = light->m();

    if (this != lightPrivate.scene)
        return false;

    auto lightIter = std::find(m_->lights->begin(), m_->lights->end(), light);
    m_->freeLights.insert(std::distance(m_->lights->begin(), lightIter));
    lightPrivate.scene = nullptr;

    m_->rootNode->m().dirtyLightIndices();
    return true;
}

std::shared_ptr<LightsList> Scene::lights() const
{
    return m_->lights;
}

} // namespace
} // namespace
