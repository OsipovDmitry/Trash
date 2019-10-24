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
          TangentBinormal)
constexpr uint32_t numAttributeComponents(VertexAttribute attrib)
{
    constexpr std::array<uint32_t, numElementsVertexAttribute()> data {
        3, 3, 2, 4
    };
    return data[castFromVertexAttribute(attrib)];
}

using VertexDeclaration = std::bitset<numElementsVertexAttribute()>;
inline uint32_t vertexDeclarationOffset(VertexDeclaration decl, VertexAttribute attrib)
{
    uint32_t offs = 0;
    for (unsigned int i = 0; i < castFromVertexAttribute(attrib); ++i)
        offs += decl.test(i) ? numAttributeComponents(castToVertexAttribute(i)) : 0;
    return offs;
}
inline uint32_t vertexDeclarationSize(VertexDeclaration decl)
{
    return vertexDeclarationOffset(decl, VertexAttribute::Count);
}

#endif // TYPES_H
