#include "object.h"
#include "scene.h"

ObjectUserData::ObjectUserData(Object& obj)
    : thisObject(obj)
{
}

Object::Object(std::shared_ptr<ObjectUserData> objectData)
    : m_scene(nullptr)
    , m_graphicsNode(std::make_shared<Node>())
{
    m_graphicsNode->setUserData(objectData);
}

std::shared_ptr<Node> Object::graphicsNode() const
{
    return m_graphicsNode;
}

void Object::update(uint64_t time, uint64_t dt)
{
    //
    doUpdate(time, dt);
}
