#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>

#include <utils/enumclass.h>

namespace trash
{
namespace core
{

ENUMCLASS(ControllerType, uint32_t,
          Core,
          Graphics,
          Audio)

ENUMCLASS(LightType, uint32_t, Point, Direction, Spot)

ENUMCLASS(TextNodeAlignment, uint8_t, Negative, Center, Positive)

namespace MouseButton {
    const uint32_t LeftButton = 1 << 0;
    const uint32_t MiddleButton = 1 << 1;
    const uint32_t RightButton = 1 << 2;
}

} // namespace
} // namespace

#endif // TYPES_H
