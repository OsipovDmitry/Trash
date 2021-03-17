#include <array>

#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <core/light.h>
#include <core/scene.h>
#include <core/scenerootnode.h>

#include "lightprivate.h"
#include "sceneprivate.h"
#include "scenerootnodeprivate.h"
#include "renderer.h"

namespace trash
{
namespace core
{

LightPrivate::LightPrivate(Light* l, LightType lightType)
    : dir(0.f, 0.f, 1.f)
    , color(1.f, 1.f, 1.f)
    , radiuses(3.f, 50.f)
    , angles(glm::pi<float>()/6.0f, glm::pi<float>()/4.0f)
    , cosAngles(glm::cos(0.5f*angles))
    , type(lightType)
    , shadowMapIsEnabled(true)
    , shadowOutside(false)
    , scene(nullptr)
    , indexInScene(static_cast<uint32_t>(-1))
    , thisLight(l)
{
}

float LightPrivate::intensity(const utils::BoundingBox &box) const
{
    float attenaution = 1.0f;

    if ((type == LightType::Point) || (type == LightType::Spot))
    {
        const glm::vec3 closestPoint = box.closestPoint(pos);
        const glm::vec3 toLight = direction(closestPoint);
        const float dist = glm::length(toLight);
        attenaution = 1.f - glm::smoothstep(radiuses.x, radiuses.x + radiuses.y, dist);
    }
    else if (type == LightType::Direction)
    {
        attenaution = std::numeric_limits<float>::max();
    }

    return attenaution;
}

glm::vec3 LightPrivate::direction(const glm::vec3& v) const
{
    glm::vec3 result(0.f, 0.f, 0.f);

    if ((type == LightType::Point) || (type == LightType::Spot))
        result =  pos - v;
    else if (type == LightType::Direction)
        result =  -dir;

    return result;
}

glm::mat4x4 LightPrivate::packParams() const
{
    return glm::mat4x4(
                glm::vec4(pos, cosAngles.x),
                glm::vec4(dir, cosAngles.y),
                glm::vec4(color, static_cast<float>(type)),
                glm::vec4(radiuses, 0.0f, shadowMapIsEnabled ? (shadowOutside ? +1.f : -1.f) : 0.0f)
                );
}

void LightPrivate::dirtyScene()
{
    if (scene)
    {
        auto& scenePrivate = scene->m();
        ScenePrivate::dirtyNodeLightIndices(*scenePrivate.rootNode);
        scenePrivate.dirtyLightParams(thisLight);
        scenePrivate.dirtyShadowMap(thisLight);
        scenePrivate.rootNode->m().dirtyLocalBoundingBox();
    }
}

} // namespace
} // namespace
