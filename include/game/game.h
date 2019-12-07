#ifndef GAME_H
#define GAME_H

#include <memory>

#include <utils/pimpl.h>
#include <core/abstractgame.h>

#include "gameglobal.h"

class GamePrivate;

class GAMESHARED_EXPORT Game : public AbstractGame
{
    PIMPL(Game)

public:
    Game();
    ~Game() override;

    void doInitialize() override;
    void doUnitialize() override;
    void doUpdate(uint64_t, uint64_t) override;
    void doMouseClick(int, int) override;

private:
    std::unique_ptr<GamePrivate> m_;

};

#endif // GAME_H
