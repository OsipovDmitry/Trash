#include <algorithm>

#include <core/core.h>
#include <core/scene.h>
#include <core/graphicscontroller.h>
#include <core/camera.h>

#include "scene.h"
#include "object.h"

namespace trash
{
namespace game
{

Scene::Scene()
    : m_camera(std::make_shared<core::Camera>())
{
    core::Core::instance().graphicsController().addCamera(0, m_camera);
}

Scene::~Scene()
{
    core::Core::instance().graphicsController().removeCamera(0);
}

std::shared_ptr<core::Camera> Scene::camera()
{
    return m_camera;
}

std::shared_ptr<const core::Camera> Scene::camera() const
{
    return m_camera;
}

void Scene::detachObject(std::shared_ptr<Object> object)
{
    assert(object->m_scene == this);

    object->m_graphicsNode->parent()->detach(object->m_graphicsNode);
    m_objects.erase(object);
    object->m_scene = nullptr;
}

void Scene::attachObject(std::shared_ptr<Object> object, std::shared_ptr<core::Node> parentNode)
{
    if (object->m_scene)
        object->m_scene->detachObject(object);

    if (!parentNode)
        parentNode = m_camera->scene()->rootNode();
    assert(parentNode->relationDegree(m_camera->scene()->rootNode()) != -1);

    m_objects.insert(object);
    object->m_scene = this;
    parentNode->attach(object->m_graphicsNode);
}

void Scene::update(uint64_t time , uint64_t dt)
{
    for (auto object : m_objects)
        object->update(time, dt);
}

std::shared_ptr<Object> Scene::findObject(core::Node *node) const
{
    if (node->relationDegree(m_camera->scene()->rootNode()) == -1)
        return nullptr;

    std::shared_ptr<ObjectUserData> data = nullptr;
    while (!data && node)
    {
        data = std::dynamic_pointer_cast<ObjectUserData>(node->userData());
        node = node->parent();
    }

    auto it = std::find_if(m_objects.begin(), m_objects.end(), [p = &data->thisObject](auto obj) {
        return p == obj.get();
    });
    assert(it != m_objects.end());

    return *it;
}

} // namespace
} // namespace
