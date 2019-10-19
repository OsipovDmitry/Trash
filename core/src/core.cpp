#include <core/core.h>
#include <core/abstractgame.h>
#include <core/graphicscontroller.h>
#include <core/audiocontroller.h>

#include "coreprivate.h"
#include "renderwidget.h"

QWidget& Core::renderWidget()
{
    return *m_->renderWidget;
}

GraphicsController& Core::graphicsController()
{
    return static_cast<GraphicsController&>(*m_->controllers[castFromControllerType(ControllerType::Graphics)]);
}

AudioController &Core::audioController()
{
    return static_cast<AudioController&>(*m_->controllers[castFromControllerType(ControllerType::Audio)]);
}

void Core::setGame(std::shared_ptr<AbstractGame> game)
{
    m_->game = game;
}

void Core::doWork(std::shared_ptr<AbstractController::Message> msg)
{
    switch (msg->type())
    {
    case ControllerMessageType::RenderWidgetWasInitialized:
    {
        m_->controllers[castFromControllerType(ControllerType::Core)] = this;
        m_->controllers[castFromControllerType(ControllerType::Graphics)] = new GraphicsController(m_->renderWidget->renderer());
        m_->controllers[castFromControllerType(ControllerType::Audio)] = new AudioController();

        if (!m_->game.expired())
        {
            m_->game.lock()->doInitialize();
        }

        break;
    }
    case ControllerMessageType::RenderWidgetWasUpdated:
    {
        auto message = msg_cast<RenderWidgetWasUpdatedMessage>(msg);
        auto updateMessage = std::make_shared<UpdateMessage>(message->time, message->dt);
        for (auto controller : m_->controllers)
        {
            controller->sendMessage(updateMessage);
            controller->process();
        }
        break;
    }
    case ControllerMessageType::RenderWidgetWasClicked:
    {
        auto message = msg_cast<RenderWidgetWasClickedMessage>(msg);
        if (!m_->game.expired())
        {
            m_->game.lock()->doMouseClick(message->x, message->y);
        }
        break;
    }
    case ControllerMessageType::RenderWidgetWasClosed:
    {
        delete static_cast<GraphicsController*>(m_->controllers[castFromControllerType(ControllerType::Graphics)]);
        delete static_cast<AudioController*>(m_->controllers[castFromControllerType(ControllerType::Audio)]);
        m_ = nullptr;

        break;
    }
    case ControllerMessageType::Update:
    {
        auto updateMessage = msg_cast<UpdateMessage>(msg);
        if (!m_->game.expired())
        {
            m_->game.lock()->doUpdate(updateMessage->time, updateMessage->dt);
        }
        break;
    }
    default:
        break;
    }
}

Core::Core()
    : AbstractController()
    , m_(std::make_unique<CorePrivate>())
{
    m_->renderWidget = new RenderWidget(*this);
}

Core::~Core()
{
}
