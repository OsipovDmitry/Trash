#include <queue>

#include <core/node.h>
#include <core/scene.h>
#include <core/camera.h>
#include <utils/frustum.h>
#include <utils/ray.h>

#include "cameraprivate.h"
#include "lightprivate.h"
#include "renderer.h"
#include "nodeprivate.h"
#include "drawables.h"

#include "sceneprivate.h"
#include <core/light.h>

namespace trash
{
namespace core
{

void CameraPrivate::setZPlanes(const std::pair<float, float>& value)
{
    zNear = value.first;
    zFar = value.second;
    projMatrixIsDirty = true;
}

const glm::mat4x4 &CameraPrivate::getViewMatrix() const
{
    return viewMatrixCache;
}

const glm::mat4x4 &CameraPrivate::getProjectionMatrix() const
{
    if (projMatrixIsDirty)
    {
        projMatrixCache = calcProjectionMatrix({zNear, zFar});
        projMatrixIsDirty = false;
    }
    return projMatrixCache;
}

glm::mat4x4 CameraPrivate::calcProjectionMatrix(const std::pair<float, float>& zDistances) const
{
    const float aspect = static_cast<float>(viewport.z) / static_cast<float>(viewport.w);

    return projMatrixAsOrtho ?
                glm::ortho(-aspect * halfHeight, +aspect * halfHeight, -halfHeight, +halfHeight, zDistances.first, zDistances.second) :
                glm::perspective(fov, aspect, zDistances.first, zDistances.second);
}

void CameraPrivate::renderScene(uint64_t time, uint64_t dt)
{
    scene->m().renderScene(time, dt, *this);
}

PickData CameraPrivate::pickScene(int32_t xi, int32_t yi)
{
    return scene->m().pickScene(xi, yi, *this);
}

} // namespace
} // namespace
