#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H

#include <memory>

#include <utils/controller.h>

#include "coreglobal.h"
#include "abstractcontroller.h"
#include "types.h"

class CORESHARED_EXPORT AudioController : public AbstractController
{
    CONTROLLER(ControllerType::Audio)

public:
    //

private:
    AudioController();
    ~AudioController() = default;

    friend class Core;
};

#endif // AUDIOCONTROLLER_H
