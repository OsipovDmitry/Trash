#ifndef DRAWABLES_H
#define DRAWABLES_H

#include <set>

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
struct RenderProgram;
struct Mesh;
struct Buffer;
struct Font;

class Drawable
{
public:
    virtual ~Drawable() = default;

    virtual LayerId layerId() const = 0;
    virtual std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const = 0;
    virtual std::shared_ptr<Mesh> mesh() const = 0;
    virtual std::shared_ptr<AbstractUniform> uniform(UniformId) const { return nullptr; }
};

class MeshDrawable : public Drawable
{
public:
    MeshDrawable(std::shared_ptr<Mesh>, std::shared_ptr<Buffer>);

    std::shared_ptr<Mesh> mesh() const override;
    std::shared_ptr<AbstractUniform> uniform(UniformId) const override;

public:
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<AbstractUniform> m_bonesBufferUniform;

};

class TexturedMeshDrawable : public MeshDrawable
{
public:
    TexturedMeshDrawable(std::shared_ptr<Mesh>,
                         std::shared_ptr<Buffer>,
                         const glm::vec4&,
                         std::shared_ptr<Texture>,
                         std::shared_ptr<Texture>,
                         std::shared_ptr<Texture>,
                         std::shared_ptr<Texture>,
                         std::shared_ptr<Texture>,
                         std::shared_ptr<LightIndicesList>);

    LayerId layerId() const override { return LayerId::OpaqueGeometry; }
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;
    std::shared_ptr<AbstractUniform> uniform(UniformId) const override;

protected:
    std::set<std::string> renderProgramDefines() const;

    mutable std::shared_ptr<RenderProgram> m_forwardRenderProgram, m_deferredRenderProgram, m_shadowProgram, m_selectionProgram;
    std::shared_ptr<AbstractUniform> m_colorUniform;
    std::shared_ptr<AbstractUniform> m_baseColorTextureUniform;
    std::shared_ptr<AbstractUniform> m_opacityTextureUniform;
    std::shared_ptr<AbstractUniform> m_normalTextureUniform;
    std::shared_ptr<AbstractUniform> m_metallicTextureUniform;
    std::shared_ptr<AbstractUniform> m_roughnessTextureUniform;
    std::shared_ptr<AbstractUniform> m_lightIndicesListUniform;
};

class SphereDrawable : public TexturedMeshDrawable
{
public:
    SphereDrawable(uint32_t, const utils::BoundingSphere&, const glm::vec4&);
};

class BoxDrawable : public TexturedMeshDrawable
{
public:
    BoxDrawable(const utils::BoundingBox&, const glm::vec4&);
};

class FrustumDrawable : public TexturedMeshDrawable
{
public:
    FrustumDrawable(const utils::Frustum&, const glm::vec4&);
};

class ConeDrawable : public TexturedMeshDrawable
{
public:
    ConeDrawable(uint32_t, float, float, const glm::vec4&);
};

class BackgroundDrawable : public Drawable
{
public:
    BackgroundDrawable(float = .0f);

    LayerId layerId() const override { return LayerId::Background; }
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;
    std::shared_ptr<Mesh> mesh() const override;
    std::shared_ptr<AbstractUniform> uniform(UniformId) const override;

protected:
    mutable std::shared_ptr<RenderProgram> m_renderProgram;
    mutable std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<AbstractUniform> m_roughnessUniform;
};

class TextDrawable : public TexturedMeshDrawable
{
public:
    TextDrawable(std::shared_ptr<Font>, const std::string&, TextNodeAlignment, TextNodeAlignment, const glm::vec4&, float);

    LayerId layerId() const override { return LayerId::TransparentGeometry; }
};

class LightDrawable : public Drawable
{
public:
    LightDrawable(LightType);

    LayerId layerId() const override { return LayerId::Lights; }
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;
    std::shared_ptr<Mesh> mesh() const override;

protected:
    std::set<std::string> renderProgramDefines() const;

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
    std::set<std::string> renderProgramDefines() const;

    mutable std::shared_ptr<RenderProgram> m_stencilRenderProgram, m_lightRenderProgram;
    mutable std::shared_ptr<Mesh> m_mesh;
};

class PostEffectDrawable : public Drawable
{
public:
    PostEffectDrawable();

    LayerId layerId() const override { return LayerId::PostEffect; }
    std::shared_ptr<RenderProgram> renderProgram(DrawableRenderProgramId) const override;
    std::shared_ptr<Mesh> mesh() const override;

protected:
    mutable std::shared_ptr<Mesh> m_mesh;
    mutable std::shared_ptr<RenderProgram> m_renderProgram;
};

} // namespace
} // namespace

#endif // DRAWABLES_H
