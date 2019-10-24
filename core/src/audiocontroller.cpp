#include <core/audiocontroller.h>

#include "abstractcontrollerprivate.h"

AudioController::AudioController()
    : AbstractController(new AbstractControllerPrivate())
{
}
