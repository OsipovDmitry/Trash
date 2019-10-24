#ifndef COREPRIVATE_H
#define COREPRIVATE_H

#include <memory>
#include <array>

#include <core/forwarddecl.h>
#include <core/types.h>

#include "abstractcontrollerprivate.h"

class RenderWidget;

class CorePrivate : public AbstractControllerPrivate
{
public:
    CorePrivate();
    ~CorePrivate() = default;

    RenderWidget* renderWidget;

    std::array<AbstractController*, numElementsControllerType()> controllers;
    std::weak_ptr<AbstractGame> game;

};

#endif // COREPRIVATE_H
