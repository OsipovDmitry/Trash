#include "object.h"
#include "scene.h"

namespace trash
{
namespace game
{

ObjectUserData::ObjectUserData(Object& obj)
    : thisObject(obj)
{
}

Object::Object(std::shared_ptr<ObjectUserData> objectData)
    : m_scene(nullptr)
    , m_graphicsNode(std::make_shared<core::Node>())
{
    m_graphicsNode->setUserData(objectData);
}

std::shared_ptr<core::Node> Object::graphicsNode() const
{
    return m_graphicsNode;
}

void Object::update(uint64_t time, uint64_t dt)
{
    //
    doUpdate(time, dt);
}

} // namespace
} // namespace
