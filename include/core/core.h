#ifndef CORE_H
#define CORE_H

#include <memory>

#include <utils/controller.h>
#include <utils/singletoon.h>
#include <utils/pimpl.h>
#include <utils/message.h>

#include <core/coreglobal.h>
#include <core/forwarddecl.h>
#include <core/abstractcontroller.h>
#include <core/types.h>

class QWidget;

namespace trash
{
namespace core
{

class CorePrivate;

class CORESHARED_EXPORT Core : public AbstractController
{
    CONTROLLER(ControllerType::Core)
    SINGLETON(Core)
    PIMPL(Core)

public:
    QWidget& renderWidget();
    GraphicsController& graphicsController();
    AudioController& audioController();

    void setGame(std::shared_ptr<AbstractGame>);

protected:
    void doWork(std::shared_ptr<Message>) override;

private:
    Core();
    ~Core() override;
};

class ResizeMessage : public AbstractController::Message
{
    MESSAGE(ControllerMessageType::Resize)
public:
    int32_t width, height;
    ResizeMessage(int32_t w, int32_t h) : AbstractController::Message(messageType()), width(w), height(h) {}
};

class UpdateMessage : public AbstractController::Message
{
    MESSAGE(ControllerMessageType::Update)
public:
    uint64_t time, dt;
    UpdateMessage(uint64_t time_, uint64_t dt_) : AbstractController::Message(messageType()), time(time_), dt(dt_) {}
};

} // namespace
} // namespace

#endif // CORE_H
