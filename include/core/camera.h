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

    void setViewport(const glm::ivec4&);
    const glm::ivec4& viewport() const;

    void setClearColor(bool, const glm::vec4& = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f));
    void setClearDepth(bool, float);

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
