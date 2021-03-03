#ifndef OBJECT_H
#define OBJECT_H

#include <utils/enumclass.h>
#include <utils/noncopyble.h>
#include <core/node.h>

namespace trash
{
namespace game
{

class Object;
class Scene;

class ObjectUserData : public core::NodeUserData
{
public:
    ObjectUserData(Object& obj) : thisObject(obj) {}

    Object &thisObject;
};

class Object : public std::enable_shared_from_this<Object>
{
    NONCOPYBLE(Object)

public:
    Object(std::shared_ptr<ObjectUserData> objectData = nullptr) : m_scene(nullptr) , m_graphicsNode(std::make_shared<core::Node>()) {
        m_graphicsNode->setUserData(objectData);
    }
    virtual ~Object() = default;

    std::shared_ptr<core::Node> graphicsNode() const  {
        return m_graphicsNode;
    }

protected:
    virtual void doUpdate(uint64_t, uint64_t) {}

protected:
    Scene *m_scene;
    std::shared_ptr<core::Node> m_graphicsNode;

    void update(uint64_t time, uint64_t dt) {
        //
        doUpdate(time, dt);
    }

    friend class Scene;
};

} // namespace
} // namespace

#endif // OBJECT_H
