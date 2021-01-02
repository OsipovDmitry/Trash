#ifndef COREPRIVATE_H
#define COREPRIVATE_H

#include <memory>
#include <array>

#include <utils/message.h>
#include <core/forwarddecl.h>
#include <core/types.h>

#include "abstractcontrollerprivate.h"
#include "typesprivate.h"

namespace trash
{
namespace core
{

class RenderWidget;

class CorePrivate : public AbstractControllerPrivate
{
public:
    CorePrivate();
    ~CorePrivate();

    RenderWidget* renderWidget;
    std::array<AbstractController*, numElementsControllerType()> controllers;
    std::weak_ptr<AbstractGame> game;

};

class ResizeMessage : public AbstractController::Message
{
    MESSAGE(ControllerMessageType::Resize)
public:
    int32_t width, height;
    ResizeMessage(int32_t w, int32_t h) : AbstractController::Message(messageType()), width(w), height(h) {}
};

class UpdateMessage : public AbstractController::Message
{
    MESSAGE(ControllerMessageType::Update)
public:
    uint64_t time, dt;
    UpdateMessage(uint64_t time_, uint64_t dt_) : AbstractController::Message(messageType()), time(time_), dt(dt_) {}
};

} // namespace
} // namespace

#endif // COREPRIVATE_H
