#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

#include <core/light.h>

#include "lightprivate.h"

namespace trash
{
namespace core
{

Light::Light(LightType lightType)
    : m_(std::make_unique<LightPrivate>(this, lightType))
{
}

Light::~Light()
{
}

LightType Light::type() const
{
    return m_->type;
}

const glm::vec3& Light::color() const
{
    return m_->color;
}

void Light::setColor(const glm::vec3& value)
{
    m_->color = value;
    m_->dirtyScene();
}

const glm::vec2& Light::radiuses() const
{
    return m_->radiuses;
}

void Light::setRadiuses(const glm::vec2& value)
{
    m_->radiuses = value;
    m_->dirtyScene();
}

const glm::vec3& Light::position() const
{
    return m_->pos;
}

void Light::setPosition(const glm::vec3& value)
{
    m_->pos = value;
    m_->dirtyScene();
}

const glm::vec3& Light::direction() const
{
    return m_->dir;
}

void Light::setDirection(const glm::vec3& value)
{
    m_->dir = glm::normalize(value);
    m_->dirtyScene();
}

const glm::vec2& Light::spotAngles() const
{
    return m_->angles;
}

void Light::setSpotAngles(const glm::vec2& value)
{
    m_->angles = value;
    m_->cosAngles = glm::cos(0.5f*m_->angles);
    m_->dirtyScene();
}

bool Light::isShadowMapEnabled() const
{
    return m_->shadowMapIsEnabled;
}

void Light::enableShadowMap(bool value)
{
    if (value != m_->shadowMapIsEnabled)
    {
        m_->shadowMapIsEnabled = value;
        m_->dirtyScene();
    }
}

bool Light::isShadowOutside() const
{
    return m_->shadowOutside;
}

void Light::enableShadowOutside(bool value)
{
    if (value != m_->shadowOutside)
    {
        m_->shadowOutside = value;
        m_->dirtyScene();
    }
}

} // namespace
} // namespace
