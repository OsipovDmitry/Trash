#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <vector>
#include <map>

#include <glm/vec3.hpp>

#include <utils/noncopyble.h>
#include <utils/pimpl.h>
#include <utils/forwarddecl.h>

#include <core/node.h>
#include <core/coreglobal.h>
#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

struct PickData
{
    std::shared_ptr<Node> node;
    glm::vec3 localCoord;
};

struct IntersectionData
{
    std::multimap<float, std::shared_ptr<Node>> nodes;
};

class ScenePrivate;

class CORESHARED_EXPORT Scene
{
    NONCOPYBLE(Scene)
    PIMPL(Scene)

public:
    Scene();
    virtual ~Scene();

    PickData pickScene(int32_t, int32_t) const;
    IntersectionData intersectScene(const utils::Ray&) const;
    IntersectionData intersectScene(const utils::Frustum&) const;

    std::shared_ptr<Node> rootNode();
    std::shared_ptr<const Node> rootNode() const;

    std::shared_ptr<Camera> camera();
    std::shared_ptr<const Camera> camera() const;

    void attachLight(std::shared_ptr<Light>);
    bool detachLight(std::shared_ptr<Light>);
    std::shared_ptr<const LightsList> lights() const;

private:
    std::unique_ptr<ScenePrivate> m_;

};

} // namespace
} // namespace

#endif // SCENE_H
