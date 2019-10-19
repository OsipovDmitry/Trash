#include <core/core.h>
#include <core/graphicscontroller.h>

#include "graphicscontrollerprivate.h"

std::shared_ptr<const Node> GraphicsController::rootNode() const
{
    return m_->rootNode;
}

std::shared_ptr<Node> GraphicsController::rootNode()
{
    return m_->rootNode;
}

void GraphicsController::doWork(std::shared_ptr<AbstractController::Message> msg)
{
    switch (msg->type())
    {
    case ControllerMessageType::Update:
    {
        auto updateMessage = msg_cast<UpdateMessage>(msg);
        m_->updateScene(updateMessage->time, updateMessage->dt);
        break;
    }
    default:
        break;
    }
}

GraphicsController::GraphicsController(Renderer& renderer)
    : AbstractController()
    , m_(std::make_unique<GraphicsControllerPrivate>(renderer))
{
}

GraphicsController::~GraphicsController()
{
}
