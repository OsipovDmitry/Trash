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

const glm::mat4x4 &CameraPrivate::viewMatrix() const
{
    return viewMatrixCache;
}

const glm::mat4x4 &CameraPrivate::projectionMatrix() const
{
    if (projMatrixDirty)
    {
        const float aspect = static_cast<float>(viewport.z) / static_cast<float>(viewport.w);

        projMatrixCache = projMatrixAsOrtho ?
                    glm::ortho(-aspect, +aspect, -1.0f, +1.0f, zNear, zFar) :
                    glm::perspective(fov, aspect, zNear, zFar);
    }
    return projMatrixCache;
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
