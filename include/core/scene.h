#ifndef SCENE_H
#define SCENE_H

#include <memory>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <utils/noncopyble.h>
#include <utils/pimpl.h>
#include <utils/forwarddecl.h>

#include <core/coreglobal.h>
#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

struct PickData
{
    std::shared_ptr<DrawableNode> node;
    glm::vec3 localCoord;
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
    utils::Ray throwRay(int32_t, int32_t) const;

    std::shared_ptr<SceneRootNode> rootNode();
    std::shared_ptr<const SceneRootNode> rootNode() const;

    const glm::mat4x4& viewMatrix() const;
    void setViewMatrix(const glm::mat4x4&);
    void setProjectionMatrixAsOrtho(float halfHeight);
    void setProjectionMatrixAsPerspective(float fov);

    void attachLight(std::shared_ptr<Light>);
    bool detachLight(std::shared_ptr<Light>);

private:
    std::unique_ptr<ScenePrivate> m_;

};

} // namespace
} // namespace

#endif // SCENE_H
