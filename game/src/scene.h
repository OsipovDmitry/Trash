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
    ~Scene();

    std::shared_ptr<core::Camera> camera();
    std::shared_ptr<const core::Camera> camera() const;

    void detachObject(std::shared_ptr<Object>);
    void attachObject(std::shared_ptr<Object>, std::shared_ptr<core::Node> = nullptr);

    void update(uint64_t, uint64_t);

    std::shared_ptr<Object> findObject(core::Node*) const;

protected:
    std::unordered_set<std::shared_ptr<Object>> m_objects;
    std::shared_ptr<core::Camera> m_camera;

};

} // namespace
} // namespace

#endif // GAMESCENE_H
