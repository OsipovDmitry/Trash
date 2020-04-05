#include <glm/gtc/matrix_transform.hpp>

#include <core/node.h>
#include <core/camera.h>
#include <core/scene.h>

#include "cameraprivate.h"

namespace trash
{
namespace core
{

Camera::Camera()
    : m_(std::make_unique<CameraPrivate>())
{
}

Camera::~Camera()
{
}

void Camera::setViewport(const glm::ivec4& size)
{
    m_->viewport = size;
    m_->projMatrixIsDirty = true;
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

const glm::mat4x4& Camera::viewMatrix() const
{
    return m_->getViewMatrix();
}

void Camera::setViewMatrix(const glm::mat4x4& value)
{
    m_->viewMatrixCache = value;
}

const glm::mat4x4& Camera::projectionMatrix() const
{
    return m_->getProjectionMatrix();
}

void Camera::setProjectionMatrixAsOrtho(float height)
{
    m_->halfHeight = height * .5f;
    m_->projMatrixAsOrtho = true;
    m_->projMatrixIsDirty = true;
}

void Camera::setProjectionMatrixAsPerspective(float fov)
{
    m_->fov = fov;
    m_->projMatrixAsOrtho = false;
    m_->projMatrixIsDirty = true;
}

} // namespace
} // namespace
