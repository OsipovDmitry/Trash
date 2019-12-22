#include <core/core.h>
#include <core/camera.h>
#include <core/graphicscontroller.h>

#include "graphicscontrollerprivate.h"

namespace trash
{
namespace core
{

void GraphicsController::addCamera(uint32_t id, std::shared_ptr<Camera> camera)
{
    auto& gcPrivate = m();
    gcPrivate.cameras.insert({id, camera});
    camera->setViewport(gcPrivate.currentViewport);
}

void GraphicsController::removeCamera(uint32_t id)
{
    m().cameras.erase(id);
}

std::shared_ptr<const Camera> GraphicsController::camera(uint32_t id) const
{
    auto& cameras = m().cameras;
    auto it = cameras.find(id);
    return (it == cameras.end()) ? nullptr : it->second;
}

std::shared_ptr<Camera> GraphicsController::camera(uint32_t id)
{
    auto& cameras = m().cameras;
    auto it = cameras.find(id);
    return (it == cameras.end()) ? nullptr : it->second;
}


void GraphicsController::doWork(std::shared_ptr<AbstractController::Message> msg)
{
    auto& gcPrivate = m();

    switch (msg->type())
    {
    case ControllerMessageType::Resize:
    {
        auto resizeMessage = msg_cast<ResizeMessage>(msg);
        gcPrivate.resize(resizeMessage->width, resizeMessage->height);
        break;
    }
    case ControllerMessageType::Update:
    {
        auto updateMessage = msg_cast<UpdateMessage>(msg);
        gcPrivate.updateScene(updateMessage->time, updateMessage->dt);
        break;
    }
    default:
        break;
    }
}

GraphicsController::GraphicsController()
    : AbstractController(new GraphicsControllerPrivate())
{
}

GraphicsController::~GraphicsController()
{
}

} // namespace
} // namespace
