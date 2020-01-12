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
    : m_(std::make_unique<ScenePrivate>())
{
    m_->rootNode = std::make_shared<SceneRootNode>(this);
    m_->dirtyAllLights = false;
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

    m_->lights.push_back(light);
    lightPrivate.scene = this;
    m_->dirtyAllLights = true;

    m_->rootNode->m().dirtyLights();
}

bool Scene::detachLight(std::shared_ptr<Light> light)
{
    auto& lightPrivate = light->m();

    if (this != lightPrivate.scene)
        return false;

    m_->lights.erase(std::find(m_->lights.begin(), m_->lights.end(), light));
    lightPrivate.scene = nullptr;
    m_->dirtyAllLights = true;

    m_->rootNode->m().dirtyLights();
    return true;
}

const LightsList& Scene::lights() const
{
    return m_->lights;
}

} // namespace
} // namespace
