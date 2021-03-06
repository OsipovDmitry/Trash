#ifndef DRAWABLES_H
#define DRAWABLES_H

#include <map>
#include <functional>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <utils/forwarddecl.h>
#include <utils/enumclass.h>
#include <core/forwarddecl.h>

#include "typesprivate.h"

namespace trash
{
namespace core
{

class AbstractUniform;
template<typename T> class Uniform;
struct RenderProgram;
struct Mesh;
struct Buffer;
struct Font;

class Drawable
{
public:
    virtual ~Drawable() = default;

    virtual LayerId layerId() const = 0;
    virtual BlendingType blendingType() const { return BlendingType::NoBlend; }
    virtual std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const = 0;
    virtual std::shared_ptr<Mesh> mesh() const = 0;
    virtual std::shared_ptr<AbstractUniform> uniform(UniformId) const { return nullptr; }

    virtual void dirtyCache() {}
};

class ParticleSystemDrawable : public Drawable
{
public:
    ParticleSystemDrawable(std::shared_ptr<Mesh>,
                           std::reference_wrapper<const ParticleType>,
                           std::reference_wrapper<const BlendingType>,
                           std::reference_wrapper<const bool>,
                           std::reference_wrapper<const float>,
                           std::shared_ptr<Texture>);

    LayerId layerId() const override;
    BlendingType blendingType() const override;
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;
    std::shared_ptr<Mesh> mesh() const override;
    std::shared_ptr<AbstractUniform> uniform(UniformId) const override;

    void dirtyCache() override;

protected:
    std::map<std::string, std::string> renderProgramDefines() const;

    std::shared_ptr<Mesh> m_mesh;
    mutable std::shared_ptr<RenderProgram> m_renderProgram;

    std::reference_wrapper<const ParticleType> m_particleType;
    std::reference_wrapper<const BlendingType> m_blendingType;
    std::reference_wrapper<const bool> m_distanceAttenuationState;
    std::shared_ptr<AbstractUniform> m_distanceAttenuationValueUniform;
    std::shared_ptr<AbstractUniform> m_opacityTextureUniform;
};

class StandardDrawable : public Drawable
{
public:
    StandardDrawable(std::shared_ptr<Mesh>,
                         std::shared_ptr<Buffer>,
                         const glm::vec4&,
                         const glm::vec2&,
                         std::shared_ptr<Texture>,
                         std::shared_ptr<Texture>,
                         std::shared_ptr<Texture>,
                         std::shared_ptr<Texture>,
                         std::shared_ptr<Texture>,
                         std::reference_wrapper<const LightIndicesList>);

    LayerId layerId() const override;
    BlendingType blendingType() const override;
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;
    std::shared_ptr<Mesh> mesh() const override;
    std::shared_ptr<AbstractUniform> uniform(UniformId) const override;
    void dirtyCache() override;

protected:
    std::map<std::string, std::string> renderProgramDefines() const;

    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<AbstractUniform> m_bonesBufferUniform;

    mutable std::shared_ptr<RenderProgram> m_forwardRenderProgram, m_deferredRenderProgram, m_shadowProgram, m_selectionProgram;
    mutable bool hasLighting;
    std::shared_ptr<Uniform<glm::vec4>> m_baseColorUniform;
    std::shared_ptr<AbstractUniform> m_metallicRoughnessUniform;
    std::shared_ptr<AbstractUniform> m_baseColorTextureUniform;
    std::shared_ptr<AbstractUniform> m_opacityTextureUniform;
    std::shared_ptr<AbstractUniform> m_normalTextureUniform;
    std::shared_ptr<AbstractUniform> m_metallicTextureUniform;
    std::shared_ptr<AbstractUniform> m_roughnessTextureUniform;
    std::shared_ptr<Uniform<std::reference_wrapper<const LightIndicesList>>> m_lightIndicesListUniform;
};

class LightDrawable : public Drawable
{
public:
    LightDrawable(LightType);

    LayerId layerId() const override { return LayerId::Lights; }
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;
    std::shared_ptr<Mesh> mesh() const override;

protected:
    std::map<std::string, std::string> renderProgramDefines() const;

    LightType m_lightType;
    mutable std::shared_ptr<RenderProgram> m_stencilRenderProgram, m_lightRenderProgram;
    mutable std::shared_ptr<Mesh> m_mesh;
};

class IBLDrawable : public Drawable
{
public:
    IBLDrawable();

    LayerId layerId() const override { return LayerId::Lights; }
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;
    std::shared_ptr<Mesh> mesh() const override;

protected:
    std::map<std::string, std::string> renderProgramDefines() const;

    mutable std::shared_ptr<RenderProgram> m_stencilRenderProgram, m_lightRenderProgram;
    mutable std::shared_ptr<Mesh> m_mesh;
};

class BackgroundDrawable : public Drawable
{
public:
    BackgroundDrawable(float = .0f);

    LayerId layerId() const override { return LayerId::Undefined; }
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;
    std::shared_ptr<Mesh> mesh() const override { return nullptr; }
    std::shared_ptr<AbstractUniform> uniform(UniformId) const override;

protected:
    mutable std::shared_ptr<RenderProgram> m_renderProgram;
    std::shared_ptr<AbstractUniform> m_metallicRoughnessUniform;
};

class SSAODrawable : public Drawable
{
public:
    SSAODrawable(float, uint32_t);

    LayerId layerId() const override { return LayerId::Undefined; }
    std::shared_ptr<Mesh> mesh() const override { return nullptr; }
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;
    std::shared_ptr<AbstractUniform> uniform(UniformId) const override;

protected:
    mutable std::shared_ptr<RenderProgram> m_renderProgram;
    std::shared_ptr<AbstractUniform> m_samplesBufferUniform;
    float m_radius;
    uint32_t m_numSamples;
};

class BloomDrawable : public Drawable
{
public:
    BloomDrawable();

    LayerId layerId() const override { return LayerId::Undefined; }
    std::shared_ptr<Mesh> mesh() const override { return nullptr; }
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;

protected:
    mutable std::shared_ptr<RenderProgram> m_renderProgram;
};

class BlurDrawable : public Drawable
{
public:
    BlurDrawable(float);

    void setType(BlurType);
    void setLevel(uint32_t);
    void setTextures(std::shared_ptr<Texture>, std::shared_ptr<Texture>);

    LayerId layerId() const override { return LayerId::Undefined; }
    std::shared_ptr<Mesh> mesh() const override { return nullptr; }
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;
    std::shared_ptr<AbstractUniform> uniform(UniformId) const override;

protected:
    mutable std::shared_ptr<RenderProgram> m_renderProgram;
    std::array<std::shared_ptr<AbstractUniform>, 2> m_sourceTextureUniform;
    std::shared_ptr<AbstractUniform> m_kernelBufferUniform;
    std::shared_ptr<Uniform<uint32_t>> m_levelUniform;
    uint32_t m_type;
    uint32_t m_radius;
};

class CombineDrawable : public Drawable
{
public:
    CombineDrawable(CombineType);

    void setTexture0(std::shared_ptr<Texture>);
    void setLevel0(uint32_t);

    void setTexture1(std::shared_ptr<Texture>);
    void setLevel1(uint32_t);

    LayerId layerId() const override { return LayerId::Undefined; }
    std::shared_ptr<Mesh> mesh() const override { return nullptr; }
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;
    std::shared_ptr<AbstractUniform> uniform(UniformId) const override;

protected:
    std::map<std::string, std::string> renderProgramDefines() const;

    mutable std::shared_ptr<RenderProgram> m_renderProgram;
    std::array<std::shared_ptr<AbstractUniform>, 2> m_sourceTextureUniform;
    std::array<std::shared_ptr<AbstractUniform>, 2> m_levelUniform;
    const CombineType m_type;
};

class PostEffectDrawable : public Drawable
{
public:
    PostEffectDrawable();

    LayerId layerId() const override { return LayerId::Undefined; }
    std::shared_ptr<Mesh> mesh() const override { return nullptr; }
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;

protected:
    mutable std::shared_ptr<RenderProgram> m_renderProgram;
};

} // namespace
} // namespace

#endif // DRAWABLES_H
