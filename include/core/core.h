#ifndef CORE_H
#define CORE_H

#include <memory>

#include <utils/controller.h>
#include <utils/singletoon.h>
#include <utils/pimpl.h>
#include <utils/message.h>

#include "coreglobal.h"
#include "forwarddecl.h"
#include "abstractcontroller.h"
#include "types.h"

class QWidget;
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

    std::unique_ptr<CorePrivate> m_;
};

class UpdateMessage : public AbstractController::Message
{
    MESSAGE(ControllerMessageType::Update)
public:
    uint64_t time, dt;
    UpdateMessage(uint64_t time_, uint64_t dt_) : AbstractController::Message(messageType()), time(time_), dt(dt_) {}
};

#endif // CORE_H
