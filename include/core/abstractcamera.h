#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

#include <glm/mat4x4.hpp>

#include <utils/noncopyble.h>

class AbstractCamera
{
    NONCOPYBLE(AbstractCamera)

public:
    virtual ~AbstractCamera() = default;

    virtual const glm::mat4x4& viewMatrix() const = 0;
};

#endif // CAMERA_H
