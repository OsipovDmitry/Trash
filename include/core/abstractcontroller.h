#ifndef ABSTRACTCONTROLLER_H
#define ABSTRACTCONTROLLER_H

#include <inttypes.h>
#include <memory>

#include <utils/noncopyble.h>
#include <utils/pimpl.h>

namespace trash
{
namespace core
{

enum class ControllerMessageType : uint32_t;

class AbstractControllerPrivate;

class AbstractController
{
    NONCOPYBLE(AbstractController)
    PIMPL(AbstractController)

public:
    class Message;

    void sendMessage(std::shared_ptr<Message>);
    void process();

protected:
    AbstractController(AbstractControllerPrivate *);
    virtual ~AbstractController() = default;

    virtual void doWork(std::shared_ptr<Message>) {}

    std::unique_ptr<AbstractControllerPrivate> m_;
};

class AbstractController::Message
{
public:
    virtual ~Message() = default;
    ControllerMessageType type() const;

protected:
    Message(ControllerMessageType);

private:
    ControllerMessageType m_type;
};

template <class T>
inline std::shared_ptr<T> msg_cast(std::shared_ptr<AbstractController::Message> msg) {
    return (msg->type() == T::messageType()) ? std::static_pointer_cast<T>(msg) : nullptr;
}

} // namespace
} // namespace

#endif // ABSTRACTCONTROLLER_H
