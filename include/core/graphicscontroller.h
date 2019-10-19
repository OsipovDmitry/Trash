#ifndef GRAPHICSCONTROLLER_H
#define GRAPHICSCONTROLLER_H

#include <memory>

#include <utils/pimpl.h>
#include <utils/controller.h>

#include "coreglobal.h"
#include "forwarddecl.h"
#include "abstractcontroller.h"
#include "types.h"

class Renderer;

class GraphicsControllerPrivate;
class CORESHARED_EXPORT GraphicsController : public AbstractController
{
    CONTROLLER(ControllerType::Graphics)
    PIMPL(GraphicsController)

public:
    std::shared_ptr<const Node> rootNode() const;
    std::shared_ptr<Node> rootNode();

protected:
    void doWork(std::shared_ptr<Message>) override;

private:
    GraphicsController(Renderer&);
    ~GraphicsController() override;

    std::unique_ptr<GraphicsControllerPrivate> m_;

    friend class Core;
};

#endif // GRAPHICSCONTROLLER_H
