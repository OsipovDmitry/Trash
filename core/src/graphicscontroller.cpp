#include <core/core.h>
#include <core/scene.h>
#include <core/camera.h>
#include <core/graphicscontroller.h>

#include "graphicscontrollerprivate.h"

namespace trash
{
namespace core
{

void GraphicsController::setMainScene(std::shared_ptr<Scene> scene)
{
    auto& gcPrivate = m();
    gcPrivate.scene = scene;
    gcPrivate.scene->camera()->setViewport(gcPrivate.currentViewport);
}

std::shared_ptr<Scene> GraphicsController::mainScene()
{
    return m().scene;
}

std::shared_ptr<const Scene> GraphicsController::mainScene() const
{
    return m().scene;
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
