#include <algorithm>

#include <core/core.h>
#include <core/scene.h>
#include <core/graphicscontroller.h>
#include <core/scenerootnode.h>

#include "scene.h"
#include "object.h"

namespace trash
{
namespace game
{

class SceneUserData : public core::NodeUserData
{
public:
    SceneUserData(Scene& sc) : thisScene(sc) {}

    Scene &thisScene;
};

Scene::Scene()
    : m_scene(std::make_shared<core::Scene>())
{
    auto& graphicsController = core::Core::instance().graphicsController();
    graphicsController.setMainScene(m_scene);

    m_scene->rootNode()->setUserData(std::make_shared<SceneUserData>(*this));
}

Scene::~Scene()
{
}

std::shared_ptr<core::Scene> Scene::graphicsScene()
{
    return m_scene;
}

std::shared_ptr<const core::Scene> Scene::graphicsScene() const
{
    return m_scene;
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
    if (object->m_scene == this)
        return;

    if (object->m_scene)
        object->m_scene->detachObject(object);

    if (!parentNode)
        parentNode = m_scene->rootNode();
    assert(parentNode->relationDegree(m_scene->rootNode()) != -1);

    m_objects.insert(object);
    object->m_scene = this;
    parentNode->attach(object->m_graphicsNode);
}

void Scene::update(uint64_t time , uint64_t dt)
{
    for (auto object : m_objects)
        object->update(time, dt);
}

std::shared_ptr<Object> Scene::findObject(std::shared_ptr<core::Node> node)
{
    std::shared_ptr<ObjectUserData> data = nullptr;
    while (!data && node)
    {
        data = std::dynamic_pointer_cast<ObjectUserData>(node->userData());
        node = node->parent();
    }

    if (!data)
        return nullptr;

    return data->thisObject.shared_from_this();
}

} // namespace
} // namespace
