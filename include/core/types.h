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

ENUMCLASS(VertexAttribute, uint32_t,
          Position,
          Normal,
          TexCoord,
          BonesIDs,
          BonesWeights,
          TangentBinormal)

namespace std
{
template<> struct hash<VertexAttribute> {
    std::size_t operator()(VertexAttribute const& key) const noexcept { return castFromVertexAttribute(key); }
};
}

#endif // TYPES_H
