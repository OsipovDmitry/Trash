#ifndef CORE_H
#define CORE_H

#include <memory>

#include <utils/controller.h>
#include <utils/singletoon.h>
#include <utils/pimpl.h>

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

} // namespace
} // namespace

#endif // CORE_H
