#include <algorithm>

#include <utils/transform.h>
#include <utils/boundingbox.h>
#include <core/modelnode.h>
#include <core/autotransformnode.h>
#include <core/textnode.h>

#include "person.h"
#include "waypoint.h"

namespace trash
{
namespace game
{

Person::Person(const std::string &modelFilename)
    : Object(std::make_shared<ObjectUserData>(*this))
{
    m_modelNode = std::make_shared<core::ModelNode>(modelFilename);
    m_graphicsNode->attach(m_modelNode);

    auto autoTransform = std::make_shared<core::AutoTransformNode>();
    autoTransform->setTransform(utils::Transform::fromTranslation(glm::vec3(0,1.1f*m_graphicsNode->boundingBox().maxPoint.y,0)) * utils::Transform::fromScale(150.0f));
    m_graphicsNode->attach(autoTransform);

    m_name = modelFilename.substr(0, modelFilename.find('.'));
    m_textNode = std::make_shared<core::TextNode>(m_name, core::TextNodeAlignment::Center, core::TextNodeAlignment::Negative);
    autoTransform->attach(m_textNode);

    m_modelNode->playAnimation("idle");
    m_state = 0;
}

void Person::moveTo(std::shared_ptr<WayPoint> value)
{
    m_state = 1;
    m_modelNode->playAnimation("walk");
    m_currentWayPoint = value;
}

void Person::wave()
{
    m_state = 2;
    m_modelNode->playAnimation("wave");
}

void Person::idle()
{
    m_state = 0;
    m_modelNode->playAnimation("idle");

}

const std::string &Person::name() const
{
    return m_name;
}

void Person::setText(const std::string& text)
{
    m_textNode->setText(text);
}

void Person::doUpdate(uint64_t time, uint64_t dt)
{
    if (m_state == 0)
    {
    }
    else if (m_state == 1)
    {
        if (!m_currentWayPoint)
        {
            idle();
        }
        else
        {
            auto curTransform = m_graphicsNode->transform();
            auto to = m_currentWayPoint->position() - curTransform.translation;
            if (glm::length(to) < 10.0f)
            {
                m_currentWayPoint = m_currentWayPoint->nextWayPoint();
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
    }
    else if (m_state == 2)
    {
        m_startWavingTime = time;
        m_state = 3;
    }
    else if (m_state == 3)
    {
        if (m_modelNode->animationTime("wave") <= time - m_startWavingTime)
        {
            moveTo(m_currentWayPoint);
            m_state = 1;
        }
    }
}

} // namespace
} // namespace
