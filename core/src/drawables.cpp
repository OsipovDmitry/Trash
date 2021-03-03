#include <glm/gtc/type_ptr.hpp>

#include <utils/frustum.h>
#include <core/types.h>

#include "renderer.h"
#include "drawables.h"
#include "resources.h"
#include "utils.h"

namespace trash
{
namespace core
{

StandardDrawable::StandardDrawable(std::shared_ptr<Mesh> mesh,
                                           std::shared_ptr<Buffer> bonesBuffer,
                                           const glm::vec4& color,
                                           const glm::vec2& metallicRoughness,
                                           std::shared_ptr<Texture> baseColorTexture,
                                           std::shared_ptr<Texture> opacityTexture,
                                           std::shared_ptr<Texture> normalTexture,
                                           std::shared_ptr<Texture> metallicTexture,
                                           std::shared_ptr<Texture> roughnessTexture,
                                           std::shared_ptr<LightIndicesList> lightIndicesList)
    : m_mesh(mesh)
    , m_bonesBufferUniform(bonesBuffer ? std::make_shared<Uniform<std::shared_ptr<Buffer>>>(bonesBuffer) : nullptr)
    , m_baseColorUniform(std::make_shared<Uniform<glm::vec4>>(color))
    , m_metallicRoughnessUniform(std::make_shared<Uniform<glm::vec2>>(metallicRoughness))
    , m_baseColorTextureUniform(baseColorTexture ? std::make_shared<Uniform<std::shared_ptr<Texture>>>(baseColorTexture) : nullptr)
    , m_opacityTextureUniform(opacityTexture ? std::make_shared<Uniform<std::shared_ptr<Texture>>>(opacityTexture) : nullptr)
    , m_normalTextureUniform(normalTexture ? std::make_shared<Uniform<std::shared_ptr<Texture>>>(normalTexture) : nullptr)
    , m_metallicTextureUniform(metallicTexture ? std::make_shared<Uniform<std::shared_ptr<Texture>>>(metallicTexture) : nullptr)
    , m_roughnessTextureUniform(roughnessTexture ? std::make_shared<Uniform<std::shared_ptr<Texture>>>(roughnessTexture) : nullptr)
    , m_lightIndicesListUniform(lightIndicesList ? std::make_shared<Uniform<std::shared_ptr<LightIndicesList>>>(lightIndicesList) : nullptr)
{
}

LayerId StandardDrawable::layerId() const
{
    LayerId result;

    if (m_baseColorUniform->get().a < .999f || m_opacityTextureUniform)
        result = LayerId::TransparentGeometry;
    else if (m_lightIndicesListUniform && m_lightIndicesListUniform->get()->isEnabled && m_mesh && m_mesh->vertexBuffer(VertexAttribute::Normal))
        result = LayerId::OpaqueGeometry;
    else
        result = LayerId::NotLightedGeometry;

    return result;
}

std::shared_ptr<RenderProgram> StandardDrawable::renderProgram(DrawableRenderProgramId id) const
{
    auto& renderer = Renderer::instance();

    std::shared_ptr<RenderProgram> result;
    switch (id)
    {
    case DrawableRenderProgramId::ForwardRender:
    {
        if (!m_forwardRenderProgram)
            m_forwardRenderProgram = renderer.loadRenderProgram(standardRenderProgramName.first, standardRenderProgramName.second, renderProgramDefines());

        result = m_forwardRenderProgram;
        break;
    }
    case DrawableRenderProgramId::DeferredGeometryPass:
    {
        if (!m_deferredRenderProgram)
            m_deferredRenderProgram = renderer.loadRenderProgram(deferredGeometryPassRenderProgramName.first, deferredGeometryPassRenderProgramName.second, renderProgramDefines());

        result = m_deferredRenderProgram;
        break;
    }
    case DrawableRenderProgramId::Shadow:
    {
        if (!m_shadowProgram)
            m_shadowProgram = renderer.loadRenderProgram(shadowRenderProgramName.first, shadowRenderProgramName.second, renderProgramDefines());

        result = m_shadowProgram;
        break;
    }
    case DrawableRenderProgramId::Selection:
    {
        if (!m_selectionProgram)
            m_selectionProgram = renderer.loadRenderProgram(idRenderProgramName.first, idRenderProgramName.second, renderProgramDefines());

        result = m_selectionProgram;
        break;
    }
    }

    return result;
}

std::shared_ptr<Mesh> StandardDrawable::mesh() const
{
    return m_mesh;
}

std::shared_ptr<AbstractUniform> StandardDrawable::uniform(UniformId id) const
{
    std::shared_ptr<AbstractUniform> result = Drawable::uniform(id);

    switch (id)
    {
    case UniformId::BonesBuffer:
    {
        result = m_bonesBufferUniform;
        break;
    }
    case UniformId::Color:
    {
        result = m_baseColorUniform;
        break;
    }
    case UniformId::MetallicRoughness:
    {
        result = m_metallicRoughnessUniform;
        break;
    }
    case UniformId::BaseColorMap:
    {
        result = m_baseColorTextureUniform;
        break;
    }
    case UniformId::OpacityMap:
    {
        result = m_opacityTextureUniform;
        break;
    }
    case UniformId::NormalMap:
    {
        result = m_normalTextureUniform;
        break;
    }
    case UniformId::MetallicMap:
    {
        result = m_metallicTextureUniform;
        break;
    }
    case UniformId::RoughnessMap:
    {
        result = m_roughnessTextureUniform;
        break;
    }
    case UniformId::LightIndicesList:
    {
        result = m_lightIndicesListUniform;
        break;
    }
    }

    return result;
}

void StandardDrawable::dirtyCache()
{
    m_forwardRenderProgram = nullptr;
    m_deferredRenderProgram = nullptr;
    m_shadowProgram = nullptr;
    m_selectionProgram = nullptr;
}

std::set<std::string> StandardDrawable::renderProgramDefines() const
{
    std::set<std::string> defines;

    bool hasPositions = m_mesh->vertexBuffer(VertexAttribute::Position) != nullptr;
    if (hasPositions)
        defines.insert("HAS_POSITIONS");

    bool hasNormals = m_mesh->vertexBuffer(VertexAttribute::Normal) != nullptr;
    if (hasNormals)
        defines.insert("HAS_NORMALS");

    if (m_mesh->vertexBuffer(VertexAttribute::TexCoord))
        defines.insert("HAS_TEXCOORDS");

    if (m_bonesBufferUniform && m_mesh->vertexBuffer(VertexAttribute::BonesIDs) && m_mesh->vertexBuffer(VertexAttribute::BonesWeights))
        defines.insert("HAS_BONES");

    if (m_mesh->vertexBuffer(VertexAttribute::Tangent))
        defines.insert("HAS_TANGENTS");

    if (m_mesh->vertexBuffer(VertexAttribute::Color))
        defines.insert("HAS_COLORS");

    if (m_lightIndicesListUniform && m_lightIndicesListUniform->get()->isEnabled)
        defines.insert("HAS_LIGHTING");

    if (m_baseColorTextureUniform)
        defines.insert("HAS_BASECOLORMAPPING");

    if (m_opacityTextureUniform)
        defines.insert("HAS_OPACITYMAPPING");

    if (m_normalTextureUniform)
        defines.insert("HAS_NORMALMAPPING");

    if (m_metallicTextureUniform)
        defines.insert("HAS_METALLICMAPPING");

    if (m_roughnessTextureUniform)
        defines.insert("HAS_ROUGHNESSMAPPING");

    return defines;
}

SphereDrawable::SphereDrawable(uint32_t segs, const utils::BoundingSphere &bs, const glm::vec4& c)
    : StandardDrawable(buildSphereMesh(segs, bs, true), nullptr, c, glm::vec2(1.f, 1.f), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)
{   
}

BoxDrawable::BoxDrawable(const utils::BoundingBox& box, const glm::vec4& c)
    : StandardDrawable(buildBoxMesh(box, true), nullptr, c, glm::vec2(1.f, 1.f), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)
{

}

FrustumDrawable::FrustumDrawable(const utils::Frustum &frustum, const glm::vec4& c)
    : StandardDrawable(buildFrustumMesh(frustum), nullptr, c, glm::vec2(1.f, 1.f), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)
{
}

ConeDrawable::ConeDrawable(uint32_t segs, float r, float l, const glm::vec4& c)
    : StandardDrawable(buildConeMesh(segs, r, l, true), nullptr, c, glm::vec2(1.f, 1.f), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)
{
}

BackgroundDrawable::BackgroundDrawable(float r)
    : Drawable()
    , m_metallicRoughnessUniform(std::make_shared<Uniform<glm::vec2>>(glm::vec2(1.f, r)))
{
}

std::shared_ptr<RenderProgram> BackgroundDrawable::renderProgram(DrawableRenderProgramId id) const
{
    if (id != DrawableRenderProgramId::ForwardRender)
        return nullptr;

    if (m_renderProgram)
        return m_renderProgram;

    auto& renderer = Renderer::instance();
    m_renderProgram = renderer.loadRenderProgram(backgroundRenderProgramName.first, backgroundRenderProgramName.second, {});

    return m_renderProgram;
}

std::shared_ptr<Mesh> BackgroundDrawable::mesh() const
{
    if (!m_mesh)
        m_mesh = buildPlaneMesh();

    return m_mesh;
}

std::shared_ptr<AbstractUniform> BackgroundDrawable::uniform(UniformId id) const
{
    std::shared_ptr<AbstractUniform> result = Drawable::uniform(id);

    switch (id)
    {
    case UniformId::MetallicRoughness:
    {
        result = m_metallicRoughnessUniform;
        break;
    }
    }

    return result;
}

TextDrawable::TextDrawable(std::shared_ptr<Font> font, const std::string& str, TextNodeAlignment alignX, TextNodeAlignment alignY, const glm::vec4& c, float lineSpacing)
    : StandardDrawable(buildTextMesh(font, str, alignX, alignY, lineSpacing), nullptr, c, glm::vec2(1.f, 1.f), nullptr, font->texture, nullptr, nullptr, nullptr, nullptr)
{
}

LightDrawable::LightDrawable(LightType type)
    : Drawable()
    , m_lightType(type)
{
}

std::shared_ptr<RenderProgram> LightDrawable::renderProgram(DrawableRenderProgramId id) const
{
    std::shared_ptr<RenderProgram> result;

    auto& renderer = Renderer::instance();

    switch (id)
    {
    case DrawableRenderProgramId::DeferredStencilPass:
    {
        if (!m_stencilRenderProgram)
            m_stencilRenderProgram = renderer.loadRenderProgram(deferredStencilPassRenderProgramName.first, deferredStencilPassRenderProgramName.second, renderProgramDefines());

        result = m_stencilRenderProgram;
        break;
    }
    case DrawableRenderProgramId::DeferredLightPass:
    {
        if (!m_lightRenderProgram)
            m_lightRenderProgram = renderer.loadRenderProgram(deferredLightPassRenderProgramName.first, deferredLightPassRenderProgramName.second, renderProgramDefines());

        result = m_lightRenderProgram;
        break;
    }
    }

    return result;
}

std::shared_ptr<Mesh> LightDrawable::mesh() const
{
    if (!m_mesh)
    {
        switch (m_lightType) {
        case LightType::Point:
        {
            m_mesh = buildSphereMesh(8, utils::BoundingSphere(glm::vec3(0.f, 0.f, 0.f), 1.f), false);
            break;
        }
        case LightType::Direction:
        {
            m_mesh = buildBoxMesh(utils::BoundingBox(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.f, 1.f, 1.f)), false);
            break;
        }
        case LightType::Spot:
        {
            m_mesh = buildConeMesh(8, 1.f, 1.f, false);
            break;
        }
        }
    }

    return m_mesh;
}

std::set<std::string> LightDrawable::renderProgramDefines() const
{
    std::set<std::string> defines;
    defines.insert("LIGHT");

    return defines;
}

IBLDrawable::IBLDrawable()
    : Drawable()
{
}

std::shared_ptr<RenderProgram> IBLDrawable::renderProgram(DrawableRenderProgramId id) const
{
    std::shared_ptr<RenderProgram> result;

    auto& renderer = Renderer::instance();

    switch (id)
    {
    case DrawableRenderProgramId::DeferredStencilPass:
    {
        if (!m_stencilRenderProgram)
            m_stencilRenderProgram = renderer.loadRenderProgram(deferredStencilPassRenderProgramName.first, deferredStencilPassRenderProgramName.second, renderProgramDefines());

        result = m_stencilRenderProgram;
        break;
    }
    case DrawableRenderProgramId::DeferredLightPass:
    {
        if (!m_lightRenderProgram)
            m_lightRenderProgram = renderer.loadRenderProgram(deferredLightPassRenderProgramName.first, deferredLightPassRenderProgramName.second, renderProgramDefines());

        result = m_lightRenderProgram;
        break;
    }
    }

    return result;
}

std::shared_ptr<Mesh> IBLDrawable::mesh() const
{
    if (!m_mesh)
    {
        m_mesh = buildBoxMesh(utils::BoundingBox(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.f, 1.f, 1.f)), false);
    }

    return m_mesh;
}

std::set<std::string> IBLDrawable::renderProgramDefines() const
{
    std::set<std::string> defines;
    defines.insert("IBL");

    return defines;
}

PostEffectDrawable::PostEffectDrawable()
{
}

std::shared_ptr<RenderProgram> PostEffectDrawable::renderProgram(DrawableRenderProgramId id) const
{
    std::shared_ptr<RenderProgram> result;

    auto& renderer = Renderer::instance();

    switch (id)
    {
    case DrawableRenderProgramId::PostEffect:
    {
        if (!m_renderProgram)
            m_renderProgram = renderer.loadRenderProgram(postEffectRenderProgramName.first, postEffectRenderProgramName.second, {});

        result = m_renderProgram;
        break;
    }
    }

    return result;
}

std::shared_ptr<Mesh> PostEffectDrawable::mesh() const
{
    static std::weak_ptr<Mesh> s_weakMesh;

    if (!m_mesh)
    {
        if (!s_weakMesh.expired())
            m_mesh = s_weakMesh.lock();
        else
        {
            m_mesh = buildPlaneMesh();
            s_weakMesh = m_mesh;
        }
    }

    return m_mesh;
}

} // namespace
} // namespace
