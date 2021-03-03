#ifndef LIGHTPRIVATE_H
#define LIGHTPRIVATE_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <core/forwarddecl.h>
#include <utils/forwarddecl.h>

namespace trash
{
namespace core
{

class LightPrivate
{
public:
    glm::vec3 pos, dir, color;
    glm::vec2 angles, cosAngles, radiuses;
    LightType type;
    bool shadowMapIsEnabled;

    Light *thisLight;

    Scene *scene;
    uint32_t indexInScene;

    LightPrivate(Light*, LightType);
    float intensity(const utils::BoundingBox&) const;
    glm::vec3 direction(const glm::vec3&) const;
    glm::mat4x4 packParams() const;

    void dirtyScene();
};

} // namespace
} // namespace

#endif // LIGHTPRIVATE_H
