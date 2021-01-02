#ifndef CAMERAPRIVATE_H
#define CAMERAPRIVATE_H

#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

class CameraPrivate
{
public:
    mutable glm::mat4x4 viewMatrixCache = glm::mat4x4(1.0f), projMatrixCache = glm::mat4x4(1.0f);

    glm::uvec2 viewportSize = glm::uvec2(512,512);

    float zNear = 0.5f, zFar = 1000.0f, fov = 1.0f, halfHeight = 1.0f;
    bool projMatrixAsOrtho = true;
    mutable bool projMatrixIsDirty = true;

    void setZPlanes(const std::pair<float, float>&);

    const glm::mat4x4& getViewMatrix() const;
    const glm::mat4x4& getProjectionMatrix() const;
    glm::mat4x4 calcProjectionMatrix(const std::pair<float, float>&) const;
};

} // namespace
} // namespace

#endif // CAMERAPRIVATE_H
