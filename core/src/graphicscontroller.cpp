#include <core/core.h>
#include <core/graphicscontroller.h>

#include "graphicscontrollerprivate.h"

std::shared_ptr<const Node> GraphicsController::rootNode() const
{
    return m().rootNode;
}

std::shared_ptr<Node> GraphicsController::rootNode()
{
    return m().rootNode;
}

void GraphicsController::doWork(std::shared_ptr<AbstractController::Message> msg)
{
    auto& gcPrivate = m();

    switch (msg->type())
    {
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

GraphicsController::GraphicsController(Renderer& renderer)
    : AbstractController(new GraphicsControllerPrivate(renderer))
{
}

GraphicsController::~GraphicsController()
{
}
