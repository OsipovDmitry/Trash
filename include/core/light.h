#ifndef LIGHT_H
#define LIGHT_H

#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <utils/noncopyble.h>
#include <utils/pimpl.h>
#include <utils/enumclass.h>

#include <core/coreglobal.h>
#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

class LightPrivate;

class CORESHARED_EXPORT Light
{
    NONCOPYBLE(Light)
    PIMPL(Light)

public:
    Light(LightType);
    virtual ~Light();

    LightType type() const;

    const glm::vec3 &color() const;
    void setColor(const glm::vec3&);

    const glm::vec2& radiuses() const;
    void setRadiuses(const glm::vec2&); // vec2(inner, attDist)

    const glm::vec3& position() const;
    void setPosition(const glm::vec3&);

    const glm::vec3& direction() const;
    void setDirection(const glm::vec3&);

    const glm::vec2& spotAngles() const;
    void setSpotAngles(const glm::vec2&); // vec2(inner, outer)

    bool isShadowMapEnabled() const;
    void enableShadowMap(bool);

    bool isShadowOutside() const;
    void enableShadowOutside(bool);

private:
    std::unique_ptr<LightPrivate> m_;

};

} // namespace
} // namespace

#endif // LIGHT_H
