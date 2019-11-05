#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <array>

#include <glm/mat4x4.hpp>

#include "plane.h"

class Frustum
{
public:
    Frustum(const glm::mat4x4& viewProjectionMatrix)
    {

    }

    std::array<Plane, 6> planes;
};

#endif // FRUSTUM_H
