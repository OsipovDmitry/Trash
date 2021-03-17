#include <utils/ray.h>

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

utils::Ray Scene::throwRay(int32_t x, int32_t y) const
{
    return m_->throwRay(x, y);
}

std::shared_ptr<SceneRootNode> Scene::rootNode()
{
    return m_->rootNode;
}

std::shared_ptr<const SceneRootNode> Scene::rootNode() const
{
    return m_->rootNode;
}

const glm::mat4x4 &Scene::viewMatrix() const
{
    return m_->viewMatrix;
}

void Scene::setViewMatrix(const glm::mat4x4& m)
{
    m_->viewMatrix = m;
}

void Scene::setProjectionMatrixAsOrtho(float halfHeight)
{
    m_->fov = halfHeight;
    m_->isPerspectiveProjection = false;
}

void Scene::setProjectionMatrixAsPerspective(float fov)
{
    m_->fov = fov;
    m_->isPerspectiveProjection = true;
}

void Scene::attachLight(std::shared_ptr<Light> light)
{
    m_->attachLight(light);
}

bool Scene::detachLight(std::shared_ptr<Light> light)
{
    return m_->detachLight(light);
}

} // namespace
} // namespace
