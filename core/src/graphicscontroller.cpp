#include <core/core.h>
#include <core/scene.h>
#include <core/graphicscontroller.h>

#include "coreprivate.h"
#include "graphicscontrollerprivate.h"
#include "typesprivate.h"

namespace trash
{
namespace core
{

void GraphicsController::setMainScene(std::shared_ptr<Scene> scene)
{
    auto& gcPrivate = m();
    gcPrivate.scene = scene;
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
    case ControllerMessageType::Update:
    {
        auto updateMessage = msg_cast<UpdateMessage>(msg);
        gcPrivate.frame(updateMessage->time, updateMessage->dt);
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
