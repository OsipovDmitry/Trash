#include <QtGui/QImage>

#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <core/textnode.h>

#include <utils/boundingsphere.h>
#include <utils/boundingbox.h>
#include <utils/frustum.h>
#include <utils/epsilon.h>

#include "utils.h"
#include "renderer.h"

namespace trash
{
namespace core
{

const float gamma = 2.2f;
const float invGamma = 1.0f / gamma;
const float dielectricSpecular = 0.04f;

glm::vec4 toLinearRGB(const glm::vec4& color)
{
    return glm::vec4(glm::pow(color.r, gamma), std::pow(color.g, gamma), std::pow(color.b, gamma), color.a);
}

glm::vec4 toSRGB(const glm::vec4& color)
{
    return glm::vec4(glm::pow(color.r, invGamma), std::pow(color.g, invGamma), std::pow(color.b, invGamma), color.a);
}


//bool diffuseSpecularGlossinessToBaseColorMetallicRoughness(
//        const QImage& diffuseMap,
//        const QImage& specularMap,
//        const QImage& glossinessMap,
//        const QImage& opacityMap,
//        QImage& baseColorMap,
//        QImage& metallicMap,
//        QImage& roughnessMap)
//{
//    static auto toVec4 = [](const QColor& color) -> glm::vec4
//    {
//        return glm::vec4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
//    };

//    static auto toQColor = [](const glm::vec4& color) -> QColor
//    {
//        return QColor::fromRgbF(color.r, color.g, color.b, color.a);
//    };

//    static auto readPixel = [toVec4](const QImage& img, double x, double y, const glm::vec4& def) -> glm::vec4
//    {
//        if (img.isNull())
//            return def;
//        return toVec4(img.pixelColor(static_cast<int>(x*(img.width()-1) + 0.5f), static_cast<int>(y*(img.height()-1) + 0.5f)));
//    };

//    static auto getPerceivedBrightness = [](const glm::vec4& color) -> float
//    {
//        return glm::sqrt(0.299f * color.r * color.r + 0.587f * color.g * color.g + 0.114f * color.b * color.b);
//    };

//    static auto solveMetallic = [](float diffuse, float specular, float oneMinusSpecularStrength) -> float
//    {
//        if (specular < dielectricSpecular)
//            return 0.0;

//        const float a = dielectricSpecular;
//        const float b = diffuse * oneMinusSpecularStrength / (1.f - dielectricSpecular) + specular - 2.f * dielectricSpecular;
//        const float c = dielectricSpecular - specular;
//        const float D = glm::max(0.f, b*b - 4*a*c);
//        return glm::clamp((-b + glm::sqrt(D)) / (2.f * a), 0.f, 1.f);
//    };

//    if (diffuseMap.isNull())
//        return false;

//    auto size = diffuseMap.size();

//    baseColorMap = QImage(size, QImage::Format_RGBA8888);
//    metallicMap = QImage(size, QImage::Format_Grayscale8);
//    roughnessMap = QImage(size, QImage::Format_Grayscale8);

//    bool hasAlpha = false;

//    for (size_t y = 0; y < size.height(); ++y)
//        for (size_t x = 0; x < size.width(); ++x)
//        {
//            glm::vec4 diffuseColor = toLinearRGB(readPixel(diffuseMap, static_cast<double>(x)/(size.width()-1), static_cast<double>(y)/(size.height()-1), glm::vec4(0.f,0.f,0.f,1.f)));
//            glm::vec4 specularColor = toLinearRGB(readPixel(specularMap, static_cast<double>(x)/(size.width()-1), static_cast<double>(y)/(size.height()-1), glm::vec4(0.f,0.f,0.f,1.f)));
//            float glossiness = readPixel(glossinessMap, static_cast<double>(x)/(size.width()-1), static_cast<double>(y)/(size.height()-1), glm::vec4(0.f,0.f,0.f,1.f)).r;
//            float opacity = readPixel(opacityMap, static_cast<double>(x)/(size.width()-1), static_cast<double>(y)/(size.height()-1), glm::vec4(1.f,1.f,1.f,1.f)).r;

//            const float oneMinusSpecularStrength = 1.f - std::max(specularColor.r, std::max(specularColor.g, specularColor.b));
//            const float metallic = solveMetallic(getPerceivedBrightness(diffuseColor), getPerceivedBrightness(specularColor), oneMinusSpecularStrength);

//            glm::vec3 baseColorFromDiffuse = glm::vec3(diffuseColor) * (oneMinusSpecularStrength / (1.f - dielectricSpecular) / glm::max(1.f - metallic, utils::epsilon));
//            glm::vec3 baseColorFromSpecular = (glm::vec3(specularColor) - glm::vec3(dielectricSpecular) * (1.f - metallic))  / glm::max(metallic, utils::epsilon);

//            glm::vec4 baseColor = glm::clamp(
//                        glm::vec4(glm::mix(baseColorFromDiffuse, baseColorFromSpecular, metallic * metallic), diffuseColor.a * opacity),
//                        glm::vec4(0.0f), glm::vec4(1.0f));

//            if (baseColor.a < 1.0f - utils::epsilon)
//                hasAlpha = true;

//            baseColorMap.setPixelColor(x, y, toQColor(toSRGB(baseColor)));
//            metallicMap.setPixelColor(x, y, toQColor(glm::vec4(metallic, metallic, metallic, metallic)));
//            roughnessMap.setPixelColor(x, y, toQColor(glm::vec4(1.f - glossiness, 1.f - glossiness, 1.f - glossiness, 1.f - glossiness)));
//        }

//    if (!hasAlpha)
//        baseColorMap = baseColorMap.convertToFormat(QImage::Format_RGB888);

//    return true;
//}


std::shared_ptr<Mesh> buildLineMesh(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& colors, bool loop)
{
    std::vector<uint32_t> indices(vertices.size());
    for (size_t i = 0; i < indices.size(); ++i)
        indices[i] = i;

    auto mesh = std::make_shared<Mesh>();
    mesh->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(vertices.size(), 3, glm::value_ptr(*vertices.data()), GL_STATIC_DRAW));
    mesh->attachIndexBuffer(std::make_shared<IndexBuffer>(loop ? GL_LINE_LOOP : GL_LINE_STRIP, indices.size(), indices.data(), GL_STATIC_DRAW));

    if (!colors.empty())
        mesh->declareVertexAttribute(VertexAttribute::Color, std::make_shared<VertexBuffer>(colors.size(), 3, glm::value_ptr(*colors.data()), GL_STATIC_DRAW));

    return mesh;
}

std::shared_ptr<Mesh> buildSphereMesh(uint32_t segs, const utils::BoundingSphere& bs, bool wf)
{
    segs = glm::max(segs, 3u);

    std::vector<glm::vec3> vertices(segs * segs);
    std::vector<uint32_t> indices(wf ? 4 * (segs-1) * segs : 6 * (segs-1) * segs);

    for (uint32_t a = 0; a < segs; ++a)
    {
        float angleA = glm::pi<float>() * (static_cast<float>(a) / (segs-1) - .5f);
        float sinA = glm::sin(angleA);
        float cosA = glm::cos(angleA);

        for (uint32_t b = 0; b < segs; ++b)
        {
            float angleB = 2.f * glm::pi<float>() * static_cast<float>(b) / segs;
            float sinB = glm::sin(angleB);
            float cosB = glm::cos(angleB);

            vertices[a * segs + b] = glm::vec3(bs.w * cosA * sinB + bs.x,
                                               bs.w * sinA + bs.y,
                                               bs.w * cosA * cosB + bs.z);

            if (a < segs-1)
            {
                indices[4 * (a*segs + b) + 0] = a * segs + b;
                indices[4 * (a*segs + b) + 1] = a * segs + (b + 1) % segs;
                indices[4 * (a*segs + b) + 2] = a * segs + b;
                indices[4 * (a*segs + b) + 3] = (a+1) * segs + b;
            }
        }
    }

    if (wf)
    {
        for (uint32_t a = 0; a < segs-1; ++a)
            for (uint32_t b = 0; b < segs; ++b)
            {
                indices[4 * (a*segs + b) + 0] = a * segs + b;
                indices[4 * (a*segs + b) + 1] = a * segs + (b + 1) % segs;
                indices[4 * (a*segs + b) + 2] = a * segs + b;
                indices[4 * (a*segs + b) + 3] = (a+1) * segs + b;
            }
    }
    else
    {
        for (uint32_t a = 0; a < segs-1; ++a)
            for (uint32_t b = 0; b < segs; ++b)
            {
                indices[6 * (a*segs + b) + 0] = a * segs + b;
                indices[6 * (a*segs + b) + 1] = a * segs + (b + 1) % segs;
                indices[6 * (a*segs + b) + 2] = (a+1) * segs + b;
                indices[6 * (a*segs + b) + 3] = a * segs + (b + 1) % segs;
                indices[6 * (a*segs + b) + 4] = (a+1) * segs + (b + 1) % segs;
                indices[6 * (a*segs + b) + 5] = (a+1) * segs + b;
            }
    }

    auto mesh = std::make_shared<Mesh>();
    mesh->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(vertices.size(), 3, glm::value_ptr(*vertices.data()), GL_STATIC_DRAW));
    mesh->attachIndexBuffer(std::make_shared<IndexBuffer>(wf ? GL_LINES : GL_TRIANGLES, indices.size(), indices.data(), GL_STATIC_DRAW));

    return mesh;
}

std::shared_ptr<Mesh> buildBoxMesh(const utils::BoundingBox& box, bool wf)
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
    static const std::vector<uint32_t> linesIndices {
        0,1, 1,2, 2,3, 3,0,
        4,5, 5,6, 6,7, 7,4,
        0,4, 1,5, 2,6, 3,7
    };
    static const std::vector<uint32_t> tringlesIndices {
        0,1,2, 0,2,3,
        4,6,5, 4,7,6,
        0,4,1, 4,5,1,
        7,3,6, 3,2,6,
        1,5,6, 1,6,2,
        4,0,3, 4,3,7,
    };

    auto mesh = std::make_shared<Mesh>();
    mesh->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(vertices.size(), 3, glm::value_ptr(*vertices.data()), GL_STATIC_DRAW));
    wf ? mesh->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_LINES, linesIndices.size(), linesIndices.data(), GL_STATIC_DRAW)) :
         mesh->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_TRIANGLES, tringlesIndices.size(), tringlesIndices.data(), GL_STATIC_DRAW));

    return mesh;
}


std::shared_ptr<Mesh> buildFrustumMesh(const utils::Frustum& frustum)
{
    static const std::vector<uint32_t> indices({ 0,1, 1,2, 2,3, 3,0, 4,5, 5,6, 6,7, 7,4, 0,4, 1,5, 2,6, 3,7 });

    auto mesh = std::make_shared<Mesh>();
    mesh->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(frustum.vertices.size(), 3, glm::value_ptr(*frustum.vertices.data()), GL_STATIC_DRAW));
    mesh->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_LINES, indices.size(), indices.data(), GL_STATIC_DRAW));

    return mesh;
}

std::shared_ptr<Mesh> buildConeMesh(uint32_t segs, float radius, float length, bool wf)
{
    std::vector<glm::vec3> vertices(segs+2);
    std::vector<uint32_t> indices(wf ? 4*segs : 6*segs);

    for (size_t i = 0; i < segs; ++i)
    {
        const float angle = static_cast<float>(i) / (segs) * glm::two_pi<float>();
        vertices[i] = glm::vec3(radius * glm::cos(angle), radius * glm::sin(angle), -length);
    }
    vertices[segs] = glm::vec3(0.f, 0.f, 0.f);
    vertices[segs+1] = glm::vec3(0.f, 0.f, -length);

    if (wf)
    {
        for (size_t i = 0; i < segs; ++i)
        {
            indices[4*i+0] = segs;
            indices[4*i+1] = i;
            indices[4*i+2] = i;
            indices[4*i+3] = (i+1)%segs;
        }
    }
    else
    {
        for (size_t i = 0; i < segs; ++i)
        {
            indices[6*i+0] = segs;
            indices[6*i+1] = i;
            indices[6*i+2] = (i+1)%segs;
            indices[6*i+3] = segs+1;
            indices[6*i+4] = (i+1)%segs;
            indices[6*i+5] = i;
        }
    }

    auto mesh = std::make_shared<Mesh>();
    mesh->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(vertices.size(), 3, glm::value_ptr(*vertices.data()), GL_STATIC_DRAW));
    mesh->attachIndexBuffer(std::make_shared<IndexBuffer>(wf ? GL_LINES : GL_TRIANGLES, indices.size(), indices.data(), GL_STATIC_DRAW));

    return mesh;
}

std::shared_ptr<Mesh> buildPlaneMesh()
{
    static const std::vector<glm::vec3> vertices {
        glm::vec3(-1.f, -1.f, 0.f),
        glm::vec3(+1.f, -1.f, 0.f),
        glm::vec3(-1.f, +1.f, 0.f),
        glm::vec3(+1.f, +1.f, 0.f)

    };
    static const std::vector<uint32_t> indices {
        0u, 1u, 2u, 3u
    };

    auto mesh = std::make_shared<Mesh>();
    mesh->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(4, 3, glm::value_ptr(*vertices.data()), GL_STATIC_DRAW));
    mesh->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_TRIANGLE_STRIP, 4, indices.data(), GL_STATIC_DRAW));

    return mesh;
}

std::shared_ptr<Mesh> buildTextMesh(std::shared_ptr<Font> font, const std::string& str, TextNodeAlignment alignX, TextNodeAlignment alignY, float lineSpacing)
{
    std::vector<glm::vec3> vertices(4*str.length());
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

        vertices.at(4 * i + 0) = glm::vec3(tmpPos, 0.f);
        vertices.at(4 * i + 1) = glm::vec3(tmpPos + glm::vec2(0.f, -chInfo->height) * texInvSize * symbolTexInvHeight, 0.f);
        vertices.at(4 * i + 2) = glm::vec3(tmpPos + glm::vec2(chInfo->width, -chInfo->height) * texInvSize * symbolTexInvHeight, 0.f);
        vertices.at(4 * i + 3) = glm::vec3(tmpPos + glm::vec2(chInfo->width, 0.f) * texInvSize * symbolTexInvHeight, 0.f);

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
        v += glm::vec3(delta, 0.f);

    auto mesh = std::make_shared<Mesh>();
    mesh->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(vertices.size(), 3, glm::value_ptr(*vertices.data()), GL_STATIC_DRAW));
    mesh->declareVertexAttribute(VertexAttribute::TexCoord, std::make_shared<VertexBuffer>(texCoords.size(), 2, glm::value_ptr(*texCoords.data()), GL_STATIC_DRAW));
    mesh->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_TRIANGLES, indices.size(), indices.data(), GL_STATIC_DRAW));

    return mesh;
}

int32_t numberOfMipmaps(int32_t width, int32_t height)
{
    return 1 + glm::floor(glm::log2(static_cast<float>(glm::max(width, height))));
}

} // namespace
} // namespace
