#ifndef GRAPHICSCONTROLLER_H
#define GRAPHICSCONTROLLER_H

#include <glm/mat4x4.hpp>

#include <utils/pimpl.h>
#include <utils/controller.h>

#include "coreglobal.h"
#include "forwarddecl.h"
#include "abstractcontroller.h"
#include "types.h"

class GraphicsControllerPrivate;
class CORESHARED_EXPORT GraphicsController : public AbstractController
{
    CONTROLLER(ControllerType::Graphics)
    PIMPL(GraphicsController)

public:
    std::shared_ptr<const Node> rootNode() const;
    std::shared_ptr<Node> rootNode();

    std::shared_ptr<const AbstractCamera> camera() const;
    void setCamera(std::shared_ptr<AbstractCamera>);

    void setViewMatrix(const glm::mat4x4&);
    void setProjectionMatrix(float, float, float);

protected:
    void doWork(std::shared_ptr<Message>) override;

private:
    GraphicsController();
    ~GraphicsController() override;

    friend class Core;
};

#endif // GRAPHICSCONTROLLER_H
