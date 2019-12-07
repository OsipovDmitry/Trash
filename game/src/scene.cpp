#include <algorithm>

#include <core/core.h>
#include <core/graphicscontroller.h>
#include <core/node.h>

#include "scene.h"
#include "object.h"

Scene::Scene()
    : m_groupNode(std::make_shared<Node>())
{
    Core::instance().graphicsController().rootNode()->attach(m_groupNode);
}

Scene::~Scene()
{
    Core::instance().graphicsController().rootNode()->detach(m_groupNode);
}

void Scene::detachObject(std::shared_ptr<Object> object)
{
    assert(object->m_scene == this);

    object->m_graphicsNode->parent()->detach(object->m_graphicsNode);
    m_objects.erase(object);
    object->m_scene = nullptr;
}

void Scene::attachObject(std::shared_ptr<Object> object, std::shared_ptr<Node> parentNode)
{
    if (object->m_scene)
        object->m_scene->detachObject(object);

    if (!parentNode)
        parentNode = m_groupNode;
    assert(parentNode->relationDegree(m_groupNode) != -1);

    m_objects.insert(object);
    object->m_scene = this;
    parentNode->attach(object->m_graphicsNode);
}

void Scene::update(uint64_t time , uint64_t dt)
{
    for (auto object : m_objects)
        object->update(time, dt);
}

std::shared_ptr<Object> Scene::findObject(Node *node) const
{
    if (node->relationDegree(m_groupNode) == -1)
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
