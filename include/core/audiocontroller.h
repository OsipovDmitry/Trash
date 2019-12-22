#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H

#include <utils/pimpl.h>
#include <utils/controller.h>

#include <core/coreglobal.h>
#include <core/abstractcontroller.h>
#include <core/types.h>

namespace trash
{
namespace core
{

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

} // namespace
} // namespace

#endif // AUDIOCONTROLLER_H
