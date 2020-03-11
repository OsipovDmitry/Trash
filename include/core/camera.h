#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

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
    std::shared_ptr<Node> node;
    glm::vec3 localCoord;
};

class CameraPrivate;

class CORESHARED_EXPORT Camera
{
    NONCOPYBLE(Camera)
    PIMPL(Camera)

public:
    Camera(std::shared_ptr<Scene> = nullptr);
    virtual ~Camera();

    PickData pickNode(int32_t, int32_t);

    void setViewport(const glm::ivec4&);
    const glm::ivec4& viewport() const;

    void setClearColor(bool, const glm::vec4& = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f));
    void setClearDepth(bool, float);

    std::shared_ptr<Scene> scene();
    std::shared_ptr<const Scene> scene() const;

    const glm::mat4x4& viewMatrix() const;
    void setViewMatrix(const glm::mat4x4&);

    const glm::mat4x4& projectionMatrix() const;
    void setProjectionMatrixAsOrtho(float);
    void setProjectionMatrixAsPerspective(float);

private:
    std::unique_ptr<CameraPrivate> m_;

};

} // namespace
} // namespace

#endif // CAMERA_H
