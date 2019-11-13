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

#include <core/node.h>
#include <utils/ray.h>
#include "nodeprivate.h"
#include "drawables.h"
std::shared_ptr<Node> GraphicsController::pickNode(int32_t xi, int32_t yi)
{
    auto& widget = Core::instance().renderWidget();

    const float x = static_cast<float>(xi) / widget.width() * 2.0f - 1.0f;
    const float y = static_cast<float>(yi) / widget.height() * 2.0f - 1.0f;

    auto modelViewMatrixInverse = glm::inverse(Renderer::instance().projectionMatrix() * m().viewMatrix);

    glm::vec4 p0 = modelViewMatrixInverse * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 p1 = modelViewMatrixInverse * glm::vec4(x, y, 1.0f, 1.0f);

    p0 /= p0.w;
    p1 /= p1.w;

    Ray ray(p0, p1-p0);

    std::dynamic_pointer_cast<SphereDrawable>(rootNode()->m().bSphereDrawable)->color_ =
            ray.intersect(rootNode()->globalTransform() * rootNode()->boundingSphere()) ?
                glm::vec4(1.f, 0.f, 0.f, 1.0f) :
                glm::vec4(0.f, 0.f, 1.f, 1.0f);

    return nullptr;
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
