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

void GraphicsControllerPrivate::frame(uint64_t time, uint64_t dt)
{
    if (scene)
        scene->m().renderScene(time, dt);
}

} // namespace
} // namespace
