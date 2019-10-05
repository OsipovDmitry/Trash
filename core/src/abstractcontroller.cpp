#include <core/abstractcontroller.h>

void AbstractController::sendMessage(std::shared_ptr<AbstractController::Message> msg)
{
    m_messages.push_back(msg);
}

void AbstractController::process()
{
    while (!m_messages.empty())
    {
        auto msg = m_messages.front();
        m_messages.pop_front();
        doWork(msg);
    }
}

AbstractController::AbstractController()
{
}

ControllerMessageType AbstractController::Message::type() const
{
    return m_type;
}

AbstractController::Message::Message(ControllerMessageType msgType)
    : m_type(msgType)
{
}
