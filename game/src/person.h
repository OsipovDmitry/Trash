#ifndef MALCOLM_H
#define MALCOLM_H

#include <string>
#include <queue>

#include <glm/vec3.hpp>

#include <utils/enumclass.h>
#include <utils/noncopyble.h>

#include <core/forwarddecl.h>

#include "object.h"

namespace trash
{
namespace game
{

ENUMCLASS(PersonTaskType, uint32_t, Idle, Wave, Travel)

class AbstractPersonTask
{
    NONCOPYBLE(AbstractPersonTask)

public:
    virtual ~AbstractPersonTask() = default;

    PersonTaskType taskType() const { return m_taskType; }

protected:
    AbstractPersonTask(PersonTaskType tt) : m_taskType(tt) {}

private:
    PersonTaskType m_taskType;

};

class PersonTaskIdle : public AbstractPersonTask
{
public:
    PersonTaskIdle() : AbstractPersonTask(PersonTaskType::Idle) {}
};

class PersonTaskWave : public AbstractPersonTask
{
public:
    PersonTaskWave() : AbstractPersonTask(PersonTaskType::Wave) {}
};

class PersonTaskTravel : public AbstractPersonTask
{
public:
    PersonTaskTravel(const glm::vec3& t) : AbstractPersonTask(PersonTaskType::Travel), target(t) {}
    virtual ~PersonTaskTravel() = default;

    virtual const std::string& animationName() const = 0;
    virtual float velocity() const = 0;

    glm::vec3 target;
};

class PersonTaskWalk : public PersonTaskTravel
{
public:
    PersonTaskWalk(const glm::vec3& t) : PersonTaskTravel(t) {}

    const std::string& animationName() const override;
    float velocity() const override;
};

class PersonTaskRun : public PersonTaskTravel
{
public:
    PersonTaskRun(const glm::vec3& t) : PersonTaskTravel(t) {}

    const std::string& animationName() const override;
    float velocity() const override;
};


class Person : public Object
{
public:
    Person(const std::string&);

    const std::string& name() const;
    void setText(const std::string&);

    void addTask(std::shared_ptr<AbstractPersonTask>);
    void clearTasks();

protected:
    void doUpdate(uint64_t, uint64_t) override;

protected:
    std::string m_name;

    std::shared_ptr<core::ModelNode> m_modelNode;
    std::shared_ptr<core::TextNode> m_textNode;

    std::queue<std::shared_ptr<AbstractPersonTask>> m_tasks;
    uint64_t m_taskProcessingStartTime;

private:
    static const float s_walkVelocity;
    static const float s_runVelocity;
};

} // namespace
} // namespace

#endif // MALCOLM_H
