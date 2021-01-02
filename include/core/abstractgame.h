#ifndef ABSTRACTGAME_H
#define ABSTRACTGAME_H

#include <inttypes.h>

#include <utils/noncopyble.h>

namespace trash
{
namespace core
{

class AbstractGame
{
    NONCOPYBLE(AbstractGame)

public:
    virtual void doInitialize() {}
    virtual void doUnitialize() {}
    virtual void doUpdate(uint64_t, uint64_t) {}
    virtual void doMouseClick(uint32_t, int, int) {}
    virtual void doMouseMove(uint32_t, int, int) {}

protected:
    AbstractGame() {}
    virtual ~AbstractGame() = default;

};

} // namespace
} // namespace

#endif // ABSTRACTGAME_H
