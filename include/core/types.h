#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>
#include <array>
#include <bitset>

#include <utils/enumclass.h>

namespace trash
{
namespace core
{

ENUMCLASS(ControllerType, uint32_t,
          Core,
          Graphics,
          Audio)

ENUMCLASS(ControllerMessageType, uint32_t,
          RenderWidgetWasInitialized,
          RenderWidgetWasResized,
          RenderWidgetWasUpdated,
          RenderWidgetWasClicked,
          RenderWidgetWasClosed,

          Resize,
          Update)

ENUMCLASS(VertexAttribute, uint32_t,
          Position,
          Normal,
          TexCoord,
          BonesIDs,
          BonesWeights,
          Tangent)

ENUMCLASS(TextureUnit, int32_t,
          BaseColor,
          Opacity,
          Normal,
          Metallic,
          Roughness)

} // namespace
} // namespace

namespace std
{
template<> struct hash<trash::core::VertexAttribute> {
    std::size_t operator()(trash::core::VertexAttribute const& key) const noexcept { return castFromVertexAttribute(key); }
};
}



#endif // TYPES_H
