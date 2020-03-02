#include <array>

#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <core/light.h>
#include <core/scene.h>

#include "lightprivate.h"
#include "sceneprivate.h"
#include "renderer.h"

namespace trash
{
namespace core
{

LightPrivate::LightPrivate(Light* l, LightType lightType)
    : dir(0.f, 0.f, 1.f)
    , color(1.f, 1.f, 1.f)
    , att(0.f, 0.f, 1.f)
    , angles(glm::pi<float>()/6.0f, glm::pi<float>()/4.0f)
    , cosAngles(glm::cos(angles))
    , shadowMapSize(512)
    , type(lightType)
    , shadowMapIsEnabled(true)
    , scene(nullptr)
    , thisLight(l)

{
    shadowMapFramebuffer = std::make_shared<Framebuffer>();
    shadowMapFramebuffer->setDrawBuffer(GL_NONE);
    shadowMapFramebuffer->setReadBuffer(GL_NONE);
}

float LightPrivate::intensity(const glm::vec3& v) const
{
    float attenaution = 1.0f;
    glm::vec3 toLight = direction(v);

    if ((type == LightType::Point) || (type == LightType::Spot))
    {
        float dist = glm::length(toLight);
        glm::vec3 l = glm::normalize(toLight);

        attenaution *= 1.0f / (att.x * dist * dist + att.y * dist + att.z);

        if (type == LightType::Spot)
        {
            float cosAngle = glm::dot(-l, dir);
            float spotAtt = (cosAngle - cosAngles.y) / (cosAngles.x - cosAngles.y);
            spotAtt = glm::clamp(spotAtt, 0.0f, 1.0f);
            attenaution *= spotAtt;
        }
    }

    return attenaution;
}

glm::vec3 LightPrivate::direction(const glm::vec3& v) const
{
    if ((type == LightType::Point) || (type == LightType::Spot))
        return pos - v;
    else if (type == LightType::Direction)
        return -dir;
    else
        return glm::vec3(0.f, 0.f, 0.f);
}

glm::mat4x4 LightPrivate::packParams() const
{
    return glm::mat4x4(
                glm::vec4(pos, cosAngles.x),
                glm::vec4(dir, cosAngles.y),
                glm::vec4(color, static_cast<float>(type)),
                glm::vec4(att, shadowMapIsEnabled ? 1.0f : 0.0f)
                );
}

glm::mat4x4 LightPrivate::matrix(float zNear, float zFar) const
{
    glm::vec3 upDir(0.0f, 1.0f, 0.0f);
    if (glm::length(glm::cross(dir, upDir)) < 1e-5f)
        upDir = glm::vec3(1.0f, 0.0f, 0.0f);

    glm::mat4x4 mat(1.0f);
    if (type == LightType::Point)
        mat = glm::perspective(angles.y, 1.0f, zNear, zFar) * glm::lookAt(pos, pos+dir, upDir);

    return mat;
}

void LightPrivate::attachShadowMap(std::shared_ptr<Texture> texture, uint32_t layer)
{
    shadowMapFramebuffer->attachDepth(texture, layer);
}

void LightPrivate::dirtyScene()
{
    if (scene)
    {
        auto& scenePrivate = scene->m();
        scenePrivate.rootNode->m().dirtyLightIndices();
        scenePrivate.dirtyLightParams(thisLight);
        scenePrivate.dirtyShadowMap(thisLight);
    }
}

} // namespace
} // namespace
