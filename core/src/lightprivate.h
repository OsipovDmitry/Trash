#ifndef LIGHTPRIVATE_H
#define LIGHTPRIVATE_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

class LightPrivate
{
public:
    LightType type;
    glm::vec3 pos, dir, color, att;
    glm::vec2 angles, cosAngles;

    Scene *scene;

    float intensity(const glm::vec3&) const;
    glm::vec3 direction(const glm::vec3&) const;
    glm::mat4x4 pack() const;

};

} // namespace
} // namespace

#endif // LIGHTPRIVATE_H
