#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <memory>
#include <unordered_set>

#include <core/forwarddecl.h>
#include <utils/noncopyble.h>

namespace trash
{
namespace game
{

class Object;

class Scene
{
    NONCOPYBLE(Scene)

public:
    Scene();
    virtual ~Scene();

    std::shared_ptr<core::Scene> graphicsScene();
    std::shared_ptr<const core::Scene> graphicsScene() const;

    void detachObject(std::shared_ptr<Object>);
    void attachObject(std::shared_ptr<Object>, std::shared_ptr<core::Node> = nullptr);

    void update(uint64_t, uint64_t);

    static std::shared_ptr<Object> findObject(std::shared_ptr<core::Node>);

protected:
    std::unordered_set<std::shared_ptr<Object>> m_objects;
    std::shared_ptr<core::Scene> m_scene;

};

} // namespace
} // namespace

#endif // GAMESCENE_H
