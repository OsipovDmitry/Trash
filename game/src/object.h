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
    ObjectUserData(Object&);

    Object &thisObject;
};

class Object
{
    NONCOPYBLE(Object)

public:
    Object(std::shared_ptr<ObjectUserData> = nullptr);
    virtual ~Object() = default;

    std::shared_ptr<core::Node> graphicsNode() const;

protected:
    virtual void doUpdate(uint64_t, uint64_t) {}

protected:
    Scene *m_scene;
    std::shared_ptr<core::Node> m_graphicsNode;

    void update(uint64_t, uint64_t);

    friend class Scene;
};

} // namespace
} // namespace

#endif // OBJECT_H
