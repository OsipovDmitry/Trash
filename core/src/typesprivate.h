#ifndef TYPESPRIVATE_H
#define TYPESPRIVATE_H

#include <array>
#include <vector>
#include <memory>
#include <inttypes.h>

#include <utils/enumclass.h>
#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

struct Texture;

using LightsList = std::vector<std::shared_ptr<Light>>;
const int32_t MAX_LIGHTS_PER_NODE = 8;

struct LightIndicesList : public std::array<int32_t, MAX_LIGHTS_PER_NODE>
{
    LightIndicesList(bool e) : std::array<int32_t, MAX_LIGHTS_PER_NODE>(), isEnabled(e) {}
    bool isEnabled;
};

ENUMCLASS(ControllerMessageType, uint32_t,
          RenderWidgetWasInitialized,
          RenderWidgetWasUpdated,
          RenderWidgetMouseClick,
          RenderWidgetMouseMove,
          RenderWidgetWasClosed,
          Update)

ENUMCLASS(VertexAttribute, uint32_t,
          Position,
          Normal,
          TexCoord,
          BonesIDs,
          BonesWeights,
          Tangent,
          Color)

ENUMCLASS(LayerId, uint32_t,
          Undefined,
          OpaqueGeometry,
          NotLightedGeometry,
          TransparentGeometry,
          Lights)

ENUMCLASS(DrawableRenderProgramId, uint32_t,
          ForwardRender,
          DeferredGeometryPass,
          DeferredStencilPass,
          DeferredLightPass,
          Shadow,
          Selection,
          PostEffect)

ENUMCLASS(UniformId, uint32_t,
          Undefined,
          NodeId,
          ModelMatrix,
          NormalMatrix,
          ViewMatrix,
          ViewMatrixInverse,
          ProjMatrix,
          ProjMatrixInverse,
          ViewProjMatrix,
          ViewProjMatrixInverse,
          ModelViewMatrix,
          NormalViewMatrix,
          ModelViewProjMatrix,
          ViewPosition,
          ViewportSize,
          IBLDiffuseMap,
          IBLSpecularMap,
          IBLSpecularMapMaxMipmapLevel,
          BrdfLutMap,
          IBLContribution,
          ShadowMaps,
          BonesBuffer,
          LightsBuffer,
          SSAOSamplesBuffer,
          BlurKernelBuffer,
          Color,
          MetallicRoughness,
          BaseColorMap,
          OpacityMap,
          NormalMap,
          MetallicMap,
          RoughnessMap,
          LightIndicesList,
          GBufferMap0,
          GBufferMap1,
          GBufferMap2,
          HDRMap,
          SSAOMap,
          SSAOContribution,
          BloomMap,
          BlurSourceMap,
          BlurOffset,
          BlurLevel,
          CombineSourceMap0,
          CombineSourceMap1,
          CombineLevel0,
          CombineLevel1)

ENUMCLASS(BlurType, uint32_t, Horizontal, Vertical)

ENUMCLASS(CombineType, uint32_t, Add)

} // namespace
} // namespace

namespace std
{
template<> struct hash<trash::core::VertexAttribute> {
    std::size_t operator()(trash::core::VertexAttribute const& key) const noexcept { return castFromVertexAttribute(key); }
};
}

namespace std
{
template<> struct hash<trash::core::UniformId> {
    std::size_t operator()(trash::core::UniformId const& key) const noexcept { return static_cast<std::size_t>(key); }
};
}

#endif // TYPESPRIVATE_H
