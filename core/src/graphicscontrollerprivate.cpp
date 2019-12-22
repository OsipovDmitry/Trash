#include <core/camera.h>

#include "graphicscontrollerprivate.h"
#include "cameraprivate.h"

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
    currentViewport = glm::ivec4(0, 0, width, height);

    for (auto& camera : cameras)
        camera.second->setViewport(currentViewport);
}

void GraphicsControllerPrivate::updateScene(uint64_t time, uint64_t dt)
{
    for (auto& camera : cameras)
        camera.second->m().renderScene(time, dt);
}

} // namespace
} // namespace
