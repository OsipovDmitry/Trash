#include <glm/gtc/matrix_transform.hpp>

#include <core/node.h>
#include <core/camera.h>
#include <core/scene.h>

#include "cameraprivate.h"

namespace trash
{
namespace core
{

Camera::Camera(std::shared_ptr<Scene> scene_)
    : m_(std::make_unique<CameraPrivate>())
{
    m_->scene = scene_;
    if (!m_->scene)
        m_->scene = std::make_shared<Scene>();
}

Camera::~Camera()
{
}

PickData Camera::pickNode(int32_t xi, int32_t yi)
{
    return m_->pickNode(xi, yi);
}

void Camera::setViewport(const glm::ivec4& size)
{
    m_->viewport = size;
    m_->projMatrixDirty = true;
}

const glm::ivec4& Camera::viewport() const
{
    return m_->viewport;
}

void Camera::setClearColor(bool state, const glm::vec4& value)
{
    m_->clearColorBuffer = state;
    m_->clearColor = value;
}

void Camera::setClearDepth(bool state, float value)
{
    m_->clearDepthBuffer = state;
    m_->clearDepth = value;
}

std::shared_ptr<Scene> Camera::scene()
{
    return m_->scene;
}

std::shared_ptr<const Scene> Camera::scene() const
{
    return m_->scene;
}

const glm::mat4x4& Camera::viewMatrix() const
{
    return m_->viewMatrix();
}

void Camera::setViewMatrix(const glm::mat4x4& value)
{
    m_->viewMatrixCache = value;
}

const glm::mat4x4& Camera::projectionMatrix() const
{
    return m_->projectionMatrix();
}

void Camera::setProjectionMatrixAsOrtho(float zNear, float zFar)
{
    m_->zNear = zNear;
    m_->zFar = zFar;
    m_->projMatrixAsOrtho = true;
    m_->projMatrixDirty = true;
}

void Camera::setProjectionMatrixAsPerspective(float fov, float zNear, float zFar)
{
    m_->fov = fov;
    m_->zNear = zNear;
    m_->zFar = zFar;
    m_->projMatrixAsOrtho = false;
    m_->projMatrixDirty = true;
}

} // namespace
} // namespace
