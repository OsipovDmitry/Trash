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

glm::mat4x4 LightPrivate::pack() const
{
    return glm::mat4x4(
                glm::vec4(pos, cosAngles.x),
                glm::vec4(dir, cosAngles.y),
                glm::vec4(color, static_cast<float>(type)),
                glm::vec4(att, 0.0f)
                );
}

} // namespace
} // namespace
