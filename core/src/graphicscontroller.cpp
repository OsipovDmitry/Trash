#include <core/core.h>
#include <core/graphicscontroller.h>

#include "coreprivate.h"
#include "graphicscontrollerprivate.h"
#include "renderwidget.h"
#include "renderer.h"

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
    m().viewMatrix = value;
    Renderer::instance().setViewMatrix(value);
}

void GraphicsController::setProjectionMatrix(float fov, float zNear, float zFar)
{
    Renderer::instance().setProjectionMatrix(fov, zNear, zFar);
}


std::shared_ptr<Node> GraphicsController::pickNode(int32_t xi, int32_t yi)
{
    return m().pickNode(xi, yi);;
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

GraphicsController::GraphicsController()
    : AbstractController(new GraphicsControllerPrivate())
{
    setViewMatrix(glm::mat4x4(1.0f));
    setProjectionMatrix(glm::pi<float>() * 0.5f, 0.5f, 10000.0f);
}

GraphicsController::~GraphicsController()
{
}
