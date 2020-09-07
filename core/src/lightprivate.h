#ifndef LIGHTPRIVATE_H
#define LIGHTPRIVATE_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <core/forwarddecl.h>
#include <utils/frustum.h>

namespace trash
{
namespace core
{

struct Framebuffer;
struct Texture;

class LightPrivate
{
public:
    glm::vec3 pos, dir, color;
    glm::vec2 angles, cosAngles, radiuses;
    LightType type;

    Scene *scene;
    Light *thisLight;

    std::shared_ptr<Framebuffer> shadowMapFramebuffer;
    glm::mat4x4 matrix;
    std::pair<float, float> zPlanes;
    bool matrixIsDirty;
    bool shadowMapIsEnabled;

    LightPrivate(Light*, LightType);
    float intensity(const glm::vec3&) const;
    glm::vec3 direction(const glm::vec3&) const;
    glm::mat4x4 packParams() const;
    glm::mat4x4 getMatrix();
    glm::mat4x4 calcMatrix(const std::pair<float, float>&) const;
    void attachShadowMap(std::shared_ptr<Texture>, uint32_t);
    void setZPlanes(const std::pair<float, float>&);

    void dirtyScene();
    void dirtyMatrix();
};

} // namespace
} // namespace

#endif // LIGHTPRIVATE_H
