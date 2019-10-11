#ifndef GRAPHICSCONTROLLER_H
#define GRAPHICSCONTROLLER_H

#include <memory>

#include <utils/pimpl.h>
#include <utils/controller.h>

#include "coreglobal.h"
#include "forwarddecl.h"
#include "abstractcontroller.h"
#include "types.h"

class CORESHARED_EXPORT GraphicsController : public AbstractController
{
    CONTROLLER(ControllerType::Graphics)

private:
    GraphicsController();
    ~GraphicsController() override;

    friend class Core;
};

#endif // GRAPHICSCONTROLLER_H
