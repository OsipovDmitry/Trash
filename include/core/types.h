#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>
#include <array>
#include <bitset>

#include <utils/enumclass.h>

ENUMCLASS(ControllerType, uint32_t,
          Core,
          Graphics,
          Audio)

ENUMCLASS(ControllerMessageType, uint32_t,
          RenderWidgetWasInitialized,
          RenderWidgetWasUpdated,
          RenderWidgetWasClicked,
          RenderWidgetWasClosed,

          Update)

#endif // TYPES_H
