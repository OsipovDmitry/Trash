#ifndef ABSTRACTGAME_H
#define ABSTRACTGAME_H

#include <inttypes.h>

#include <utils/noncopyble.h>

class AbstractGame
{
    NONCOPYBLE(AbstractGame)

public:
    virtual void doInitialize() {}
    virtual void doUpdate(uint64_t, uint64_t) {}
    virtual void doMouseClick(int, int) {}

protected:
    AbstractGame() {}
    virtual ~AbstractGame() = default;

};

#endif // ABSTRACTGAME_H
