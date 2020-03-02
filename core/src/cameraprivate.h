#ifndef CAMERAPRIVATE_H
#define CAMERAPRIVATE_H

#include <memory>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

struct PickData;

class CameraPrivate
{
public:
    mutable glm::mat4x4 viewMatrixCache = glm::mat4x4(1.0f), projMatrixCache = glm::mat4x4(1.0f);

    std::shared_ptr<Scene> scene;

    glm::ivec4 viewport = glm::ivec4(0,0,512,512);

    float zNear = 0.5f, zFar = 1000.0f, fov = 1.0f;
    bool projMatrixAsOrtho = true, projMatrixDirty = true;

    bool clearColorBuffer = true, clearDepthBuffer = true;
    glm::vec4 clearColor = glm::vec4(.5f, .5f, 1.f, 1.f);
    float clearDepth = 1.0f;

    const glm::mat4x4& viewMatrix() const;
    const glm::mat4x4& projectionMatrix() const;

    void renderScene(uint64_t, uint64_t);
    PickData pickScene(int32_t, int32_t);
};

} // namespace
} // namespace

#endif // CAMERAPRIVATE_H
