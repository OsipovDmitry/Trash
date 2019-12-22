#include <core/audiocontroller.h>

#include "abstractcontrollerprivate.h"

namespace trash
{
namespace core
{

AudioController::AudioController()
    : AbstractController(new AbstractControllerPrivate())
{
}

} // namespace
} // namespace
