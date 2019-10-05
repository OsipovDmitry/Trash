#ifndef COREPRIVATE_H
#define COREPRIVATE_H

#include <memory>
#include <array>

#include <core/types.h>

class RenderWidget;
class AbstractController;
class AbstractGame;

class CorePrivate
{
public:
    ~CorePrivate() = default;

    RenderWidget* renderWidget;
    std::array<AbstractController*, numElementsControllerType()> controllers;
    std::weak_ptr<AbstractGame> game;

};

#endif // COREPRIVATE_H
