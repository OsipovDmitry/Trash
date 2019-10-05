#ifndef GRAPHICSCONTROLLER_H
#define GRAPHICSCONTROLLER_H

#include <memory>

#include <utils/controller.h>

#include "coreglobal.h"
#include "abstractcontroller.h"
#include "types.h"

class CORESHARED_EXPORT GraphicsController : public AbstractController
{
    CONTROLLER(ControllerType::Graphics)

public:
    //

private:
    GraphicsController();
    ~GraphicsController();

    friend class Core;
};

#endif // GRAPHICSCONTROLLER_H
