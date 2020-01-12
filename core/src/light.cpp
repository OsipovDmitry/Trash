#include <glm/gtc/constants.hpp>

#include <core/light.h>
#include <core/scene.h>
#include <core/node.h>

#include "lightprivate.h"
#include "sceneprivate.h"
#include "nodeprivate.h"

namespace trash
{
namespace core
{

Light::Light(LightType lightType)
    : m_(std::make_unique<LightPrivate>())
{
    m_->type = lightType;
    m_->pos = glm::vec3();
    m_->dir = glm::vec3(0.f, 0.f, 1.f);
    m_->color = glm::vec3(1.f, 1.f, 1.f);
    m_->att = glm::vec3(0.f, 0.f, 1.f);
    m_->angles = glm::vec2(glm::pi<float>()/6.0f, glm::pi<float>()/4.0f);
    m_->cosAngles = glm::cos(m_->angles);
    m_->scene = nullptr;
}

Light::~Light()
{

}

LightType Light::type() const
{
    return m_->type;
}

void Light::setType(LightType value)
{
    m_->type = value;

    if (m_->scene)
    {
        auto& scenePrivate = m_->scene->m();
        scenePrivate.rootNode->m().dirtyLights();
        scenePrivate.dirtyLight(this);
    }
}

const glm::vec3& Light::color() const
{
    return m_->color;
}

void Light::setColor(const glm::vec3& value)
{
    m_->color = value;

    if (m_->scene)
    {
        auto& scenePrivate = m_->scene->m();
        //scenePrivate.rootNode->m().dirtyLights();
        //scenePrivate.dirtyLight(this);
    }
}

const glm::vec3& Light::attenuation() const
{
    return m_->att;
}

void Light::setAttenuation(const glm::vec3& value)
{
    m_->att = value;

    if (m_->scene)
    {
        auto& scenePrivate = m_->scene->m();
        scenePrivate.rootNode->m().dirtyLights();
        scenePrivate.dirtyLight(this);
    }
}

const glm::vec3& Light::position() const
{
    return m_->pos;
}

void Light::setPosition(const glm::vec3& value)
{
    m_->pos = value;

    if (m_->scene)
    {
        auto& scenePrivate = m_->scene->m();
        scenePrivate.rootNode->m().dirtyLights();
        scenePrivate.dirtyLight(this);
    }
}

const glm::vec3& Light::direction() const
{
    return m_->dir;
}

void Light::setDirection(const glm::vec3& value)
{
    m_->dir = glm::normalize(value);

    if (m_->scene)
    {
        auto& scenePrivate = m_->scene->m();
        scenePrivate.rootNode->m().dirtyLights();
        scenePrivate.dirtyLight(this);
    }
}

const glm::vec2& Light::spotAngles() const
{
    return m_->angles;
}

void Light::setSpotAngles(const glm::vec2& value)
{
    m_->angles = value;
    m_->cosAngles = glm::cos(m_->angles);

    if (m_->scene)
    {
        auto& scenePrivate = m_->scene->m();
        scenePrivate.rootNode->m().dirtyLights();
        scenePrivate.dirtyLight(this);
    }
}

} // namespace
} // namespace
