#include <core/core.h>
#include <core/abstractgame.h>
#include <core/graphicscontroller.h>
#include <core/audiocontroller.h>

#include "coreprivate.h"
#include "renderwidget.h"
#include "importexport.h"

namespace trash
{
namespace core
{

QWidget& Core::renderWidget()
{
    return *m().renderWidget;
}

GraphicsController& Core::graphicsController()
{
    return static_cast<GraphicsController&>(*m().controllers[castFromControllerType(ControllerType::Graphics)]);
}

AudioController &Core::audioController()
{
    return static_cast<AudioController&>(*m().controllers[castFromControllerType(ControllerType::Audio)]);
}

void Core::setGame(std::shared_ptr<AbstractGame> game)
{
    m().game = game;
}

void Core::doWork(std::shared_ptr<AbstractController::Message> msg)
{
    auto& corePrivate = m();

    switch (msg->type())
    {
    case ControllerMessageType::RenderWidgetWasInitialized:
    {
        corePrivate.controllers[castFromControllerType(ControllerType::Core)] = this;
        corePrivate.controllers[castFromControllerType(ControllerType::Graphics)] = new GraphicsController();
        corePrivate.controllers[castFromControllerType(ControllerType::Audio)] = new AudioController();

        if (!corePrivate.game.expired())
        {
            corePrivate.game.lock()->doInitialize();
        }

        break;
    }
    case ControllerMessageType::RenderWidgetWasResized:
    {
        auto message = msg_cast<RenderWidgetWasResizedMessage>(msg);
        auto resizeMessage = std::make_shared<ResizeMessage>(message->width, message->height);
        for (auto controller : corePrivate.controllers)
        {
            controller->sendMessage(resizeMessage);
        }
        break;
    }
    case ControllerMessageType::RenderWidgetWasUpdated:
    {
        auto message = msg_cast<RenderWidgetWasUpdatedMessage>(msg);
        auto updateMessage = std::make_shared<UpdateMessage>(message->time, message->dt);
        for (auto controller : corePrivate.controllers)
        {
            controller->sendMessage(updateMessage);
            controller->process();
        }
        break;
    }
    case ControllerMessageType::RenderWidgetMouseClick:
    {
        auto message = msg_cast<RenderWidgetMouseClickMessage>(msg);
        if (!corePrivate.game.expired())
        {
            corePrivate.game.lock()->doMouseClick(message->buttonMask, message->x, message->y);
        }
        break;
    }
    case ControllerMessageType::RenderWidgetMouseMove:
    {
        auto message = msg_cast<RenderWidgetMouseMoveMessage>(msg);
        if (!corePrivate.game.expired())
        {
            corePrivate.game.lock()->doMouseMove(message->buttonMask, message->x, message->y);
        }
        break;
    }
    case ControllerMessageType::RenderWidgetWasClosed:
    {
        if (!corePrivate.game.expired())
        {
            corePrivate.game.lock()->doUnitialize();
        }
        delete static_cast<GraphicsController*>(corePrivate.controllers[castFromControllerType(ControllerType::Graphics)]);
        delete static_cast<AudioController*>(corePrivate.controllers[castFromControllerType(ControllerType::Audio)]);

        break;
    }
    case ControllerMessageType::Update:
    {
        auto updateMessage = msg_cast<UpdateMessage>(msg);
        if (!corePrivate.game.expired())
        {
            corePrivate.game.lock()->doUpdate(updateMessage->time, updateMessage->dt);
        }
        break;
    }
    default:
        break;
    }
}

Core::Core()
    : AbstractController(new CorePrivate())
{
    m().renderWidget = new RenderWidget(*this);
}

Core::~Core()
{
}

} // namespace
} // namespace
