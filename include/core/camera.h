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

class CameraPrivate;

class CORESHARED_EXPORT Camera
{
    NONCOPYBLE(Camera)
    PIMPL(Camera)

public:
    Camera();
    virtual ~Camera();

    void setViewportSize(const glm::uvec2&);
    const glm::uvec2& viewportSize() const;

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
