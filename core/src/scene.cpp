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

PickData Scene::pickScene(int32_t x, int32_t y) const
{
    return m_->pickScene(x, y);
}

std::shared_ptr<Node> Scene::rootNode()
{
    return m_->rootNode;
}

std::shared_ptr<const Node> Scene::rootNode() const
{
    return m_->rootNode;
}

std::shared_ptr<Camera> Scene::camera()
{
    return m_->camera;
}

std::shared_ptr<const Camera> Scene::camera() const
{
    return m_->camera;
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
