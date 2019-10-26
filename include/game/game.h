#ifndef GAME_H
#define GAME_H

#include <core/abstractgame.h>

#include "gameglobal.h"

class GAMESHARED_EXPORT Game : public AbstractGame
{
public:
    void doInitialize() override;
    void doUpdate(uint64_t, uint64_t) override;
    void doMouseClick(int, int) override;

    Game();

private:
    uint64_t m_currTime = 0;
    uint64_t m_resetTime = 0;
};

#endif // GAME_H
