#ifndef GRAPHICSCONTROLLER_H
#define GRAPHICSCONTROLLER_H

#include <utils/pimpl.h>
#include <utils/controller.h>

#include <core/coreglobal.h>
#include <core/forwarddecl.h>
#include <core/abstractcontroller.h>
#include <core/types.h>

namespace trash
{
namespace core
{

class GraphicsControllerPrivate;

class CORESHARED_EXPORT GraphicsController : public AbstractController
{
    CONTROLLER(ControllerType::Graphics)
    PIMPL(GraphicsController)

public:
    void setMainScene(std::shared_ptr<Scene>);
    std::shared_ptr<Scene> mainScene();
    std::shared_ptr<const Scene> mainScene() const;

protected:
    void doWork(std::shared_ptr<Message>) override;

private:
    GraphicsController();
    ~GraphicsController() override;

    friend class Core;
};

} // namespace
} // namespace

#endif // GRAPHICSCONTROLLER_H
