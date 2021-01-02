#include <core/camera.h>
#include <core/scene.h>

#include "graphicscontrollerprivate.h"
#include "sceneprivate.h"

namespace trash
{
namespace core
{

GraphicsControllerPrivate::GraphicsControllerPrivate()
    : AbstractControllerPrivate()
{
}

GraphicsControllerPrivate::~GraphicsControllerPrivate()
{
}

void GraphicsControllerPrivate::resize(int32_t width, int32_t height)
{
    currentViewportSize = glm::uvec2(width, height);

    if (scene)
        scene->camera()->setViewportSize(currentViewportSize);
}

void GraphicsControllerPrivate::updateScene(uint64_t time, uint64_t dt)
{
    if (scene)
        scene->m().renderScene(time, dt);
}

} // namespace
} // namespace
