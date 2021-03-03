#include <algorithm>

#include <utils/transform.h>
#include <utils/boundingbox.h>
#include <core/modelnode.h>
#include <core/autotransformnode.h>
#include <core/textnode.h>

#include "person.h"

namespace trash
{
namespace game
{

const std::string &PersonTaskWalk::animationName() const
{
    static const std::string name("walk");
    return name;
}

float PersonTaskWalk::velocity() const
{
    return 1.1f;
}

const std::string &PersonTaskRun::animationName() const
{
    static const std::string name("run");
    return name;
}

float PersonTaskRun::velocity() const
{
    return 3.3f;
}

const float Person::s_walkVelocity = 1.1f;
const float Person::s_runVelocity = 3.3f;

Person::Person(const std::string &modelFilename)
    : Object(std::make_shared<ObjectUserData>(*this))
    , m_taskProcessingStartTime(static_cast<uint64_t>(-1))
{
    m_modelNode = std::make_shared<core::ModelNode>(modelFilename);
    m_modelNode->setTransform(utils::Transform::fromScale(1.f / 200.f));

    m_graphicsNode->attach(m_modelNode);

    auto autoTransform = std::make_shared<core::AutoTransformNode>();
    autoTransform->setTransform(utils::Transform::fromTranslation(glm::vec3(0,1.1f*m_graphicsNode->boundingBox().maxPoint.y,0)) * utils::Transform::fromScale(.75f));
    m_graphicsNode->attach(autoTransform);

    m_name = modelFilename.substr(0, modelFilename.find('.'));
    m_textNode = std::make_shared<core::TextNode>(m_name, core::TextNodeAlignment::Center, core::TextNodeAlignment::Negative);
    m_textNode->setColor(glm::vec4(0.2f, 0.2f, 1.0f, 0.6f));
    autoTransform->attach(m_textNode);
}

const std::string &Person::name() const
{
    return m_name;
}

void Person::setText(const std::string& text)
{
    m_textNode->setText(text);
}

void Person::addTask(std::shared_ptr<AbstractPersonTask> task)
{
    m_tasks.push(task);
}

void Person::clearTasks()
{
    while(!m_tasks.empty())
        m_tasks.pop();
}

void Person::doUpdate(uint64_t time, uint64_t dt)
{
    bool isProcessed = false;

    while (!isProcessed)
    {
        if (m_tasks.empty())
        {
            m_tasks.push(std::make_shared<PersonTaskIdle>());
            if (m_taskProcessingStartTime == static_cast<uint64_t>(-1))
                m_taskProcessingStartTime = time;
        }

        auto currentTask = m_tasks.front();
        switch (currentTask->taskType())
        {
        case PersonTaskType::Idle:
        {
            if (m_tasks.size() > 1)
            {
                m_tasks.pop();
                m_taskProcessingStartTime = time;
            }
            else
            {
                const uint64_t animFrame = time - m_taskProcessingStartTime;
                m_modelNode->setAnimationFrame("idle", animFrame);
                isProcessed = true;
            }
            break;
        }
        case PersonTaskType::Wave:
        {
            const uint64_t animFrame = time - m_taskProcessingStartTime;
            const uint64_t animTime =  m_modelNode->animationTime("wave");
            if (animFrame < animTime)
            {
                m_modelNode->setAnimationFrame("wave", animFrame);
                isProcessed = true;
            }
            else
            {
                m_tasks.pop();
                dt = animFrame - animTime;
                m_taskProcessingStartTime = time - dt;
            }
            break;
        }
        case PersonTaskType::Travel:
        {
            auto travelTask = std::dynamic_pointer_cast<PersonTaskTravel>(currentTask);
            auto currentTransform = m_graphicsNode->transform();
            const glm::vec3 walkDir = travelTask->target - currentTransform.translation;
            const uint64_t requiredTime = static_cast<uint64_t>(1000 * glm::length(walkDir) / travelTask->velocity() + .5f);
            if (requiredTime >= dt)
            {
                const glm::vec3 z(glm::normalize(walkDir));
                const glm::vec3 y(0.0f, 1.0f, 0.0f);
                const glm::vec3 x(glm::cross(y, z));
                currentTransform.translation += travelTask->velocity() * dt * .001f * z;
                currentTransform.rotation = glm::quat_cast(glm::mat3x3(x, y, z));
                m_graphicsNode->setTransform(currentTransform);

                const uint64_t animFrame = time - m_taskProcessingStartTime;
                m_modelNode->setAnimationFrame(travelTask->animationName(), animFrame);

                isProcessed = true;
            }
            else
            {
                const glm::vec3 z(glm::normalize(walkDir));
                const glm::vec3 y(0.0f, 1.0f, 0.0f);
                const glm::vec3 x(glm::cross(y, z));
                currentTransform.translation = travelTask->target;
                currentTransform.rotation = glm::quat_cast(glm::mat3x3(x, y, z));
                m_graphicsNode->setTransform(currentTransform);

                dt -= requiredTime;
                m_taskProcessingStartTime = time - dt;
                m_tasks.pop();
            }
            break;
        }
        default:
        {
            break;
        }
        }
    }
}

} // namespace
} // namespace
