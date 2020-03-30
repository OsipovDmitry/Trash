#include <algorithm>

#include <utils/transform.h>
#include <core/modelnode.h>
#include <core/textnode.h>

#include "person.h"

namespace trash
{
namespace game
{

Person::Person(const std::string &modelFilename)
    : Object(std::make_shared<ObjectUserData>(*this))
{
    m_modelNode = std::make_shared<core::ModelNode>(modelFilename);
    m_graphicsNode->attach(m_modelNode);

//    auto textNode = std::make_shared<core::TextNode>(modelFilename.substr(0, modelFilename.find('.')), core::TextNodeAlignment::Center, core::TextNodeAlignment::Negative);
//    textNode->setTransform(utils::Transform::fromTranslation(glm::vec3(0,350.f,0)) * utils::Transform::fromScale(210.0f));
//    m_graphicsNode->attach(textNode);

    m_modelNode->playAnimation("idle");
    m_state = 0;
}

void Person::moveTo(const glm::vec3 &t)
{
    m_state = 1;
    m_modelNode->playAnimation("walk");
    m_target = t;
}

void Person::wave()
{
    m_state = 2;
}

void Person::doUpdate(uint64_t time, uint64_t dt)
{
    if (m_state == 0)
    {
    }
    else if (m_state == 1)
    {
        auto curTransform = m_graphicsNode->transform();
        auto to = m_target - curTransform.translation;
        if (glm::length(to) < 10.0f)
        {
            m_modelNode->playAnimation("idle");
            m_state = 0;
        }
        else
        {
            glm::vec3 z = glm::normalize(to);
            glm::vec3 y(0.0f, 1.0f, 0.0f);
            glm::vec3 x = glm::cross(y, z);
            curTransform.translation += 330.0f * dt * 0.001f * z;
            curTransform.rotation = glm::quat_cast(glm::mat3x3(x, y, z));
            m_graphicsNode->setTransform(curTransform);
        }
    }
    else if (m_state == 2)
    {
        m_startWavingTime = time;
        m_state = 3;
        m_modelNode->playAnimation("wave");
    }
    else if (m_state == 3)
    {
        if (m_modelNode->animationTime("wave") <= time - m_startWavingTime)
        {
            m_modelNode->playAnimation("idle");
            m_state = 0;
        }
    }
}

} // namespace
} // namespace
