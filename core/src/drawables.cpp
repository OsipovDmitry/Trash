#include <utils/frustum.h>

#include "renderer.h"
#include "drawables.h"
#include "resources.h"

namespace trash
{
namespace core
{

SelectionDrawable::SelectionDrawable(uint32_t id_)
    : id(id_)
{
}

uint32_t SelectionDrawable::colorToId(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return static_cast<uint32_t>(r | (g << 8) | (b << 16) | (a << 24));
}

glm::vec4 SelectionDrawable::idToColor(uint32_t id)
{
    return glm::vec4((id & 0xff) / 255.f, ((id >> 8) & 0xff) / 255.f, ((id >> 16) & 0xff) / 255.f, ((id >> 24) & 0xff) / 255.f);
}

MeshDrawable::MeshDrawable(std::shared_ptr<RenderProgram> p, std::shared_ptr<Mesh> m, std::shared_ptr<Buffer> bb)
    : program(p)
    , geometry(m)
    , bonesBuffer(bb)
{
}

std::shared_ptr<RenderProgram> MeshDrawable::renderProgram() const
{
    return program;
}

std::shared_ptr<Mesh> MeshDrawable::mesh() const
{
    return geometry;
}

std::shared_ptr<SelectionDrawable> MeshDrawable::selectionDrawable(uint32_t id) const
{
    return std::make_shared<SelectionMeshDrawable>(geometry, bonesBuffer, id);
}

void MeshDrawable::prerender() const
{
    Drawable::prerender();

    auto& renderer = Renderer::instance();

    GLuint bonesBufferIndex = program->uniformBufferIndexByName("u_bonesBuffer");
    if (bonesBuffer && (bonesBufferIndex != (GLuint)-1))
    {
        program->setUniformBufferBinding(bonesBufferIndex, 0);
        renderer.bindUniformBuffer(bonesBuffer, 0);
    }
}

SelectionMeshDrawable::SelectionMeshDrawable(std::shared_ptr<Mesh> m, std::shared_ptr<Buffer> bb, uint32_t id)
    : SelectionDrawable(id)
    , geometry(m)
    , bonesBuffer(bb)

{
    auto& renderer = Renderer::instance();

    if (bonesBuffer)
        program = renderer.loadRenderProgram(coloreredMeshRenderProgramName.first, coloreredMeshRenderProgramName.second);
    else
        program = renderer.loadRenderProgram(coloreredStaticMeshRenderProgramName.first, coloreredStaticMeshRenderProgramName.second);
}

std::shared_ptr<RenderProgram> SelectionMeshDrawable::renderProgram() const
{
    return program;
}

std::shared_ptr<Mesh> SelectionMeshDrawable::mesh() const
{
    return geometry;
}

std::shared_ptr<SelectionDrawable> SelectionMeshDrawable::selectionDrawable(uint32_t) const
{
    return nullptr;
}

void SelectionMeshDrawable::prerender() const
{
    auto& renderer = Renderer::instance();

    GLuint bonesBufferIndex = program->uniformBufferIndexByName("u_bonesBuffer");
    if (bonesBuffer && (bonesBufferIndex != (GLuint)-1))
    {
        program->setUniformBufferBinding(bonesBufferIndex, 0);
        renderer.bindUniformBuffer(bonesBuffer, 0);
    }

    program->setUniform(program->uniformLocation("u_color"), idToColor(id));
}

ColoredMeshDrawable::ColoredMeshDrawable(std::shared_ptr<Mesh> m, const glm::vec4& c, std::shared_ptr<Buffer> bb)
    : MeshDrawable(nullptr, m, bb)
    , color(c)
{
    auto& renderer = Renderer::instance();

    if (bonesBuffer)
        program = renderer.loadRenderProgram(coloreredMeshRenderProgramName.first, coloreredMeshRenderProgramName.second);
    else
        program = renderer.loadRenderProgram(coloreredStaticMeshRenderProgramName.first, coloreredStaticMeshRenderProgramName.second);
}

void ColoredMeshDrawable::prerender() const
{
    MeshDrawable::prerender();

    program->setUniform(program->uniformLocation("u_color"), color);
}

TexturedMeshDrawable::TexturedMeshDrawable(std::shared_ptr<Mesh> m,
                                           std::shared_ptr<Texture> bct,
                                           std::shared_ptr<Texture> ot,
                                           std::shared_ptr<Texture> nt,
                                           std::shared_ptr<Texture> mt,
                                           std::shared_ptr<Texture> rt,
                                           bool mrw,
                                           std::shared_ptr<Buffer> bb,
                                           const LightIndicesList& ll)
    : MeshDrawable(nullptr, m, bb)
    , baseColorTexture(bct)
    , opacityTexture(ot)
    , normalTexture(nt)
    , metallicOrSpecTexture(mt)
    , roughOrGlossTexture(rt)
    , isMetallicRoughWorkflow(mrw)
    , lightsList(ll)
{
    auto& renderer = Renderer::instance();

    if (bonesBuffer)
        program = renderer.loadRenderProgram(texturedMeshRenderProgramName.first, texturedMeshRenderProgramName.second);
    else
        program = renderer.loadRenderProgram(texturedStaticMeshRenderProgramName.first, texturedStaticMeshRenderProgramName.second);

    if (!baseColorTexture)
        baseColorTexture = renderer.loadTexture(standardDiffuseTextureName);

    if (!opacityTexture)
        opacityTexture = renderer.loadTexture(1.0f);

    if (!normalTexture)
        normalTexture = renderer.loadTexture(glm::vec3(.5f, .5f, 1.f));

    if (!metallicOrSpecTexture)
        metallicOrSpecTexture = renderer.loadTexture(0.0f);

    if (!roughOrGlossTexture)
        roughOrGlossTexture = renderer.loadTexture(0.6f);
}

void TexturedMeshDrawable::prerender() const
{
    MeshDrawable::prerender();

    auto& renderer = Renderer::instance();

    program->setUniform(program->uniformLocation("u_baseColorMap"), castFromTextureUnit(TextureUnit::BaseColor));
    renderer.bindTexture(baseColorTexture, castFromTextureUnit(TextureUnit::BaseColor));

    program->setUniform(program->uniformLocation("u_opacityMap"), castFromTextureUnit(TextureUnit::Opacity));
    renderer.bindTexture(opacityTexture, castFromTextureUnit(TextureUnit::Opacity));

    program->setUniform(program->uniformLocation("u_normalMap"), castFromTextureUnit(TextureUnit::Normal));
    renderer.bindTexture(normalTexture, castFromTextureUnit(TextureUnit::Normal));

    program->setUniform(program->uniformLocation("u_metallicMap"), castFromTextureUnit(TextureUnit::Metallic));
    renderer.bindTexture(metallicOrSpecTexture, castFromTextureUnit(TextureUnit::Metallic));

    program->setUniform(program->uniformLocation("u_roughnessMap"), castFromTextureUnit(TextureUnit::Roughness));
    renderer.bindTexture(roughOrGlossTexture, castFromTextureUnit(TextureUnit::Roughness));

    program->setUniform(program->uniformLocation("u_isMetallicRoughWorkflow"), isMetallicRoughWorkflow ? 1 : 0);

    for (size_t i = 0; i < MAX_LIGHTS_PER_NODE; ++i)
    {
        program->setUniform(program->uniformLocation("u_lightIndices["+std::to_string(i)+"]"), lightsList[i]);
    }
}

SphereDrawable::SphereDrawable(uint32_t segs, const utils::BoundingSphere &bs, const glm::vec4& c)
    : ColoredMeshDrawable(nullptr, c, nullptr)
{
    std::vector<glm::vec3> vertices((segs+1) * (segs * segs));
    std::vector<uint32_t> indices(4 * (segs * segs * segs));

    for (uint32_t a = 0; a <= segs; ++a)
    {
        float angleA = glm::pi<float>() * (static_cast<float>(a) / segs - .5f);
        float sinA = glm::sin(angleA);
        float cosA = glm::cos(angleA);

        for (uint32_t b = 0; b < segs*segs; ++b)
        {
            float angleB = 2.f * glm::pi<float>() * static_cast<float>(b) / (segs*segs - 1);
            float sinB = glm::sin(angleB);
            float cosB = glm::cos(angleB);

            vertices[a * segs * segs + b] = glm::vec3(bs.w * cosA * sinB + bs.x,
                                                      bs.w * sinA + bs.y,
                                                      bs.w * cosA * cosB + bs.z);

            if ((a < segs) && (b < (segs*segs - 1)))
            {
                indices[4 * (a*(segs*segs) + b) + 0] = a*(segs*segs) + b;
                indices[4 * (a*(segs*segs) + b) + 1] = a*(segs*segs) + b + 1;
                indices[4 * (a*(segs*segs) + b) + 2] = a*(segs*segs) + b;
                indices[4 * (a*(segs*segs) + b) + 3] = (a+1)*(segs*segs) + b;
            }
        }
    }

    geometry = std::make_shared<Mesh>();
    geometry->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(vertices.size(), 3, reinterpret_cast<float*>(vertices.data()), GL_STATIC_DRAW));
    geometry->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_LINES, indices.size(), indices.data(), GL_STATIC_DRAW));
}

FrustumDrawable::FrustumDrawable(const utils::Frustum &frustum, const glm::vec4& c)
    : ColoredMeshDrawable(nullptr, c, nullptr)
{
    static auto intersectPlanes = [](const utils::Plane& p0, const utils::Plane& p1, const utils::Plane& p2) -> glm::vec3 {
        glm::vec3 bxc = glm::cross(p1.normal(), p2.normal());
        glm::vec3 cxa = glm::cross(p2.normal(), p0.normal());
        glm::vec3 axb = glm::cross(p0.normal(), p1.normal());
        glm::vec3 r = -p0.dist() * bxc - p1.dist() * cxa - p2.dist() * axb;
        return -r * (1.0f / glm::dot(p0.normal(), bxc));
    };

    std::vector<glm::vec3> vertices({
        intersectPlanes(frustum.planes[4], frustum.planes[0], frustum.planes[2]),
        intersectPlanes(frustum.planes[4], frustum.planes[1], frustum.planes[2]),
        intersectPlanes(frustum.planes[4], frustum.planes[1], frustum.planes[3]),
        intersectPlanes(frustum.planes[4], frustum.planes[0], frustum.planes[3]),
        intersectPlanes(frustum.planes[5], frustum.planes[0], frustum.planes[2]),
        intersectPlanes(frustum.planes[5], frustum.planes[1], frustum.planes[2]),
        intersectPlanes(frustum.planes[5], frustum.planes[1], frustum.planes[3]),
        intersectPlanes(frustum.planes[5], frustum.planes[0], frustum.planes[3]) });

    std::vector<uint32_t> indices({ 0,1, 1,2, 2,3, 3,0, 4,5, 5,6, 6,7, 7,4, 0,4, 1,5, 2,6, 3,7 });

    geometry = std::make_shared<Mesh>();
    geometry->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(vertices.size(), 3, reinterpret_cast<float*>(vertices.data()), GL_STATIC_DRAW));
    geometry->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_LINES, indices.size(), indices.data(), GL_STATIC_DRAW));
}

} // namespace
} // namespace
