#include <core/abstractcontroller.h>

#include "abstractcontrollerprivate.h"

namespace trash
{
namespace core
{

void AbstractController::sendMessage(std::shared_ptr<AbstractController::Message> msg)
{
    m_->messages.push_back(msg);
}

AbstractController::AbstractController(AbstractControllerPrivate *p)
    : m_(std::unique_ptr<AbstractControllerPrivate>(p))
{
}

void AbstractController::process()
{
    while (!m_->messages.empty())
    {
        auto msg = m_->messages.front();
        m_->messages.pop_front();
        doWork(msg);
    }
}

ControllerMessageType AbstractController::Message::type() const
{
    return m_type;
}

AbstractController::Message::Message(ControllerMessageType msgType)
    : m_type(msgType)
{
}

} // namespace
} // namespace
