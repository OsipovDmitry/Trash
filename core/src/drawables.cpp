#include <glm/gtc/type_ptr.hpp>

#include <utils/frustum.h>
#include <core/textnode.h>

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

std::shared_ptr<ShadowDrawable> MeshDrawable::shadowDrawable() const
{
    return std::make_shared<ShadowMeshDrawable>(geometry, bonesBuffer);
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

void SelectionMeshDrawable::prerender() const
{
    auto& renderer = Renderer::instance();

    GLuint bonesBufferIndex = program->uniformBufferIndexByName("u_bonesBuffer");
    if (bonesBuffer && (bonesBufferIndex != static_cast<GLuint>(-1)))
    {
        program->setUniformBufferBinding(bonesBufferIndex, 0);
        renderer.bindUniformBuffer(bonesBuffer, 0);
    }

    program->setUniform(program->uniformLocation("u_color"), idToColor(id));
}

ShadowMeshDrawable::ShadowMeshDrawable(std::shared_ptr<Mesh> m, std::shared_ptr<Buffer> bb)
    : ShadowDrawable()
    , geometry(m)
    , bonesBuffer(bb)
{
    auto& renderer = Renderer::instance();

    if (bonesBuffer)
        program = renderer.loadRenderProgram(shadowMeshRenderProgramName.first, shadowMeshRenderProgramName.second);
    else
        program = renderer.loadRenderProgram(shadowStaticMeshRenderProgramName.first, shadowStaticMeshRenderProgramName.second);
}

std::shared_ptr<RenderProgram> ShadowMeshDrawable::renderProgram() const
{
    return program;
}

std::shared_ptr<Mesh> ShadowMeshDrawable::mesh() const
{
    return geometry;
}

void ShadowMeshDrawable::prerender() const
{
    auto& renderer = Renderer::instance();

    GLuint bonesBufferIndex = program->uniformBufferIndexByName("u_bonesBuffer");
    if (bonesBuffer && (bonesBufferIndex != static_cast<GLuint>(-1)))
    {
        program->setUniformBufferBinding(bonesBufferIndex, 0);
        renderer.bindUniformBuffer(bonesBuffer, 0);
    }
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
                                           std::shared_ptr<LightIndicesList> ll)
    : MeshDrawable(nullptr, m, bb)
    , baseColorTexture(bct)
    , opacityTexture(ot)
    , normalTexture(nt)
    , metallicOrSpecTexture(mt)
    , roughOrGlossTexture(rt)
    , lightIndicesList(ll)
    , isMetallicRoughWorkflow(mrw)
{
    auto& renderer = Renderer::instance();

    if (bonesBuffer)
        program = renderer.loadRenderProgram(texturedMeshRenderProgramName.first, texturedMeshRenderProgramName.second);
    else
        program = renderer.loadRenderProgram(texturedStaticMeshRenderProgramName.first, texturedStaticMeshRenderProgramName.second);

    if (!baseColorTexture)
        baseColorTexture = renderer.loadTexture(standardDiffuseTextureName);

    if (!opacityTexture)
        opacityTexture = renderer.createTexture2D(GL_R16F, 1, 1, GL_RED, GL_FLOAT, &standardOpacityTexture.second, standardOpacityTexture.first);

    if (!normalTexture)
        normalTexture = renderer.createTexture2D(GL_RGB16F, 1, 1, GL_RGB, GL_FLOAT, glm::value_ptr(standardNormalTexture.second), standardNormalTexture.first);

    if (!metallicOrSpecTexture)
        metallicOrSpecTexture = renderer.createTexture2D(GL_R16F, 1, 1, GL_RED, GL_FLOAT, &standardMetallicTexture.second, standardMetallicTexture.first);

    if (!roughOrGlossTexture)
        roughOrGlossTexture = renderer.createTexture2D(GL_R16F, 1, 1, GL_RED, GL_FLOAT, &standardRoughnessTexture.second, standardRoughnessTexture.first);
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

    for (size_t i = 0; i < lightIndicesList->size(); ++i)
    {
        program->setUniform(program->uniformLocation("u_lightIndices["+std::to_string(i)+"]"), lightIndicesList->at(i));
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

BoxDrawable::BoxDrawable(const utils::BoundingBox& box, const glm::vec4& c)
    : ColoredMeshDrawable(nullptr, c, nullptr)
{
    std::vector<glm::vec3> vertices {
                glm::vec3(box.minPoint.x, box.minPoint.y, box.minPoint.z),
                glm::vec3(box.minPoint.x, box.maxPoint.y, box.minPoint.z),
                glm::vec3(box.maxPoint.x, box.maxPoint.y, box.minPoint.z),
                glm::vec3(box.maxPoint.x, box.minPoint.y, box.minPoint.z),
                glm::vec3(box.minPoint.x, box.minPoint.y, box.maxPoint.z),
                glm::vec3(box.minPoint.x, box.maxPoint.y, box.maxPoint.z),
                glm::vec3(box.maxPoint.x, box.maxPoint.y, box.maxPoint.z),
                glm::vec3(box.maxPoint.x, box.minPoint.y, box.maxPoint.z),
    };
    std::vector<uint32_t> indices {
        0,1, 1,2, 2,3, 3,0,
        4,5, 5,6, 6,7, 7,4,
        0,4, 1,5, 2,6, 3,7
    };

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

BackgroundDrawable::BackgroundDrawable()
{
    static const std::vector<float> vertices {
        -1.f, -1.f,
        +1.f, -1.f,
        -1.f, +1.f,
        +1.f, +1.f,

    };
    static const std::vector<uint32_t> indices {
        0, 1, 2, 3
    };

    geometry = std::make_shared<Mesh>();
    geometry->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(4, 2, vertices.data(), GL_STATIC_DRAW));
    geometry->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_TRIANGLE_STRIP, 4, indices.data(), GL_STATIC_DRAW));

    auto& renderer = Renderer::instance();
    program = renderer.loadRenderProgram(backgroundRenderProgramName.first, backgroundRenderProgramName.second);
}

std::shared_ptr<RenderProgram> BackgroundDrawable::renderProgram() const
{
    return program;
}

std::shared_ptr<Mesh> BackgroundDrawable::mesh() const
{
    return geometry;
}

void BackgroundDrawable::prerender() const
{
    Drawable::prerender();

    auto& renderer = Renderer::instance();
    const auto& view = renderer.viewMatrix();
    const auto& proj = renderer.projectionMatrix();
    program->setUniform(program->uniformLocation("u_backgroundMatrix"), glm::mat4x4(glm::inverse(glm::mat3x3(view))) * glm::inverse(proj));
    program->setUniform(program->uniformLocation("u_roughness"), 0.05f);
}

TextDrawable::TextDrawable(std::shared_ptr<Font> font, const std::string& str, TextNodeAlignment alignX, TextNodeAlignment alignY, const glm::vec4& color_, float lineSpacing)
    : fontMap(font->texture),
      color(color_)
{
    std::vector<glm::vec2> vertices(4*str.length());
    std::vector<glm::vec2> texCoords(4*str.length());
    std::vector<uint32_t> indices(6*str.length());

    glm::vec2 pos(.0f, -1.f);
    const glm::vec2 texInvSize = glm::vec2(1.f, 1.f) / glm::vec2(font->width, font->height);
    const float symbolTexInvHeight = static_cast<float>(font->height) / static_cast<float>(font->size);

    glm::vec2 leftTop(.0f, .0f), bottomRight(.0f, .0f);

    for (size_t i = 0; i < str.length(); ++i)
    {
        const char ch = str.at(i);

        if (ch == '\n')
        {
            pos.x = .0f;
            pos.y -= lineSpacing;
            continue;
        }

        auto chIt = font->characters.find(ch);

        if (chIt == font->characters.end())
            chIt = font->characters.find('?');

        const auto& chInfo = chIt->second;

        const glm::vec2 tmpPos = pos + glm::vec2(chInfo->originX, chInfo->originY) * texInvSize * symbolTexInvHeight;

        vertices.at(4 * i + 0) = tmpPos;
        vertices.at(4 * i + 1) = tmpPos + glm::vec2(0.f, -chInfo->height) * texInvSize * symbolTexInvHeight;
        vertices.at(4 * i + 2) = tmpPos + glm::vec2(chInfo->width, -chInfo->height) * texInvSize * symbolTexInvHeight;
        vertices.at(4 * i + 3) = tmpPos + glm::vec2(chInfo->width, 0.f) * texInvSize * symbolTexInvHeight;

        texCoords.at(4 * i + 0) = glm::vec2(chInfo->x, chInfo->y) * texInvSize;
        texCoords.at(4 * i + 1) = glm::vec2(chInfo->x, chInfo->y + chInfo->height) * texInvSize;
        texCoords.at(4 * i + 2) = glm::vec2(chInfo->x + chInfo->width, chInfo->y + chInfo->height) * texInvSize;
        texCoords.at(4 * i + 3) = glm::vec2(chInfo->x + chInfo->width, chInfo->y) * texInvSize;

        indices.at(6 * i + 0) = i * 4 + 0;
        indices.at(6 * i + 1) = i * 4 + 1;
        indices.at(6 * i + 2) = i * 4 + 2;
        indices.at(6 * i + 3) = i * 4 + 0;
        indices.at(6 * i + 4) = i * 4 + 2;
        indices.at(6 * i + 5) = i * 4 + 3;

        if (vertices.at(4 * i + 0).x < leftTop.x) leftTop.x = vertices.at(4 * i + 0).x;
        if (vertices.at(4 * i + 0).y > leftTop.y) leftTop.y = vertices.at(4 * i + 0).y;
        if (vertices.at(4 * i + 2).x > bottomRight.x) bottomRight.x = vertices.at(4 * i + 2).x;
        if (vertices.at(4 * i + 2).y < bottomRight.y) bottomRight.y = vertices.at(4 * i + 2).y;

        pos += glm::vec2(chInfo->advance, .0f) * texInvSize * symbolTexInvHeight;
    }

    const glm::vec2 size(bottomRight.x - leftTop.x, leftTop.y - bottomRight.y);

    glm::vec2 delta;
    if (alignX == TextNodeAlignment::Center) delta.x = -0.5f * size.x;
    else if (alignX == TextNodeAlignment::Positive) delta.x = -size.x;

    if (alignY == TextNodeAlignment::Center) delta.y = 0.5f * size.y;
    else if (alignY == TextNodeAlignment::Negative) delta.y = size.y;

    for (auto& v : vertices)
        v += delta;

    geometry = std::make_shared<Mesh>();
    geometry->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(vertices.size(), 2, glm::value_ptr(*vertices.data()), GL_STATIC_DRAW));
    geometry->declareVertexAttribute(VertexAttribute::TexCoord, std::make_shared<VertexBuffer>(texCoords.size(), 2, glm::value_ptr(*texCoords.data()), GL_STATIC_DRAW));
    geometry->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_TRIANGLES, indices.size(), indices.data(), GL_STATIC_DRAW));

    auto& renderer = Renderer::instance();
    program = renderer.loadRenderProgram(textRenderProgramName.first, textRenderProgramName.second);
}

std::shared_ptr<RenderProgram> TextDrawable::renderProgram() const
{
    return program;
}

std::shared_ptr<Mesh> TextDrawable::mesh() const
{
    return geometry;
}

void TextDrawable::prerender() const
{
    Drawable::prerender();

    auto& renderer = Renderer::instance();

    program->setUniform(program->uniformLocation("u_fontMap"), castFromTextureUnit(TextureUnit::BaseColor));
    renderer.bindTexture(fontMap, castFromTextureUnit(TextureUnit::BaseColor));

    program->setUniform(program->uniformLocation("u_color"), color);
}

} // namespace
} // namespace
