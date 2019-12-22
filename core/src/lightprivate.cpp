#include <array>

#include <glm/geometric.hpp>

#include <core/light.h>

#include "lightprivate.h"

namespace trash
{
namespace core
{

float LightPrivate::intensity(const glm::vec3& v) const
{
    static const std::array<float, numElementsLightType()> s_isDirectionLightType { 0.0, 0.0, 1.0, 0.0 };
    static const std::array<float, numElementsLightType()> s_distanceAttenuationLightType { 0.0, 1.0, 0.0, 1.0 };
    static const std::array<float, numElementsLightType()> s_spotAttenuationLightType { 0.0, 0.0, 0.0, 1.0 };

    auto toLight = glm::mix(pos - v, -dir, s_isDirectionLightType[castFromLightType(type)]);
    float distToLight = glm::length(toLight);
    toLight = glm::normalize(toLight);

    float distAtt = att.x * distToLight * distToLight + att.y * distToLight + att.z;
    distAtt = distAtt * s_distanceAttenuationLightType[castFromLightType(type)];

    float spotAtt = (glm::dot(toLight, dir) - cosAngles.x) / (cosAngles.y - cosAngles.x);
    spotAtt = 1.0f - glm::clamp(spotAtt, 0.0f, 1.0f);
    spotAtt = spotAtt * s_spotAttenuationLightType[castFromLightType(type)];

    return distAtt * spotAtt;
}

glm::mat4x4 LightPrivate::pack() const
{
    return glm::mat4x4(
                glm::vec4(pos, cosAngles.x),
                glm::vec4(dir, cosAngles.y),
                glm::vec4(color, static_cast<float>(type)),
                glm::vec4(att, 0.0)
                );
}

} // namespace
} // namespace
