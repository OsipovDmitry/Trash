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

void Camera::setViewportSize(const glm::uvec2& size)
{
    m_->viewportSize = size;
    m_->projMatrixIsDirty = true;
}

const glm::uvec2& Camera::viewportSize() const
{
    return m_->viewportSize;
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
