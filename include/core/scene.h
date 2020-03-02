#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <vector>

#include <utils/noncopyble.h>
#include <utils/pimpl.h>

#include <core/node.h>
#include <core/coreglobal.h>
#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

class ScenePrivate;

class CORESHARED_EXPORT Scene
{
    NONCOPYBLE(Scene)
    PIMPL(Scene)

public:
    Scene();
    virtual ~Scene();

    std::shared_ptr<Node> rootNode();
    std::shared_ptr<const Node> rootNode() const;

    void attachLight(std::shared_ptr<Light>);
    bool detachLight(std::shared_ptr<Light>);
    std::shared_ptr<LightsList> lights() const;

private:
    std::unique_ptr<ScenePrivate> m_;

};

} // namespace
} // namespace

#endif // SCENE_H
