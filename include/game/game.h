#ifndef GAME_H
#define GAME_H

#include <core/abstractgame.h>

#include "gameglobal.h"

class GAMESHARED_EXPORT Game : public AbstractGame
{
public:
    void doUpdate(uint64_t, uint64_t) override;

    Game();
};

#endif // GAME_H
