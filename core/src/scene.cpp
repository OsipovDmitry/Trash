#include <core/scene.h>

#include "sceneprivate.h"

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
    m_->attachLight(light);
}

bool Scene::detachLight(std::shared_ptr<Light> light)
{
    return m_->detachLight(light);
}

std::shared_ptr<LightsList> Scene::lights() const
{
    return m_->lights;
}

} // namespace
} // namespace
