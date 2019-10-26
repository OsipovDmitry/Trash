#include <core/core.h>
#include <core/graphicscontroller.h>

#include "renderer.h"
#include "graphicscontrollerprivate.h"

std::shared_ptr<const Node> GraphicsController::rootNode() const
{
    return m().rootNode;
}

std::shared_ptr<Node> GraphicsController::rootNode()
{
    return m().rootNode;
}

std::shared_ptr<const AbstractCamera> GraphicsController::camera() const
{
    return m().camera;
}

void GraphicsController::setCamera(std::shared_ptr<AbstractCamera> value)
{
    m().camera = value;
}

void GraphicsController::setViewMatrix(const glm::mat4x4& value)
{
    m().renderer.setViewMatrix(value);
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
