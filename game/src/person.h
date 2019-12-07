#ifndef MALCOLM_H
#define MALCOLM_H

#include <glm/vec3.hpp>

#include "object.h"

class ModelNode;

class Person : public Object
{
public:
    Person(const std::string&);

    void moveTo(const glm::vec3&);
    void wave();

protected:
    void doUpdate(uint64_t, uint64_t) override;

protected:
    std::shared_ptr<ModelNode> m_modelNode;

    int m_state; // 0 - idle, 1 - walk, 2 - start wave, 3 - wave
    glm::vec3 m_target;
    uint64_t m_startWavingTime;

};

#endif // MALCOLM_H