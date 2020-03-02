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

struct Framebuffer;
struct Texture;

class LightPrivate
{
public:
    glm::vec3 pos, dir, color, att;
    glm::vec2 angles, cosAngles;
    std::shared_ptr<Framebuffer> shadowMapFramebuffer;
    int32_t shadowMapSize;
    LightType type;
    bool shadowMapIsEnabled;

    Scene *scene;
    Light *thisLight;

    LightPrivate(Light*, LightType);
    float intensity(const glm::vec3&) const;
    glm::vec3 direction(const glm::vec3&) const;
    glm::mat4x4 packParams() const;
    glm::mat4x4 matrix(float, float) const;
    void attachShadowMap(std::shared_ptr<Texture>, uint32_t);

    void dirtyScene();

};

} // namespace
} // namespace

#endif // LIGHTPRIVATE_H
