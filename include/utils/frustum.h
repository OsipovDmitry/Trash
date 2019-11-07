#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <array>

#include <glm/mat4x4.hpp>

#include "plane.h"
#include "boundingsphere.h"

struct Frustum
{
public:
    Frustum(const glm::mat4x4& vp)
    {
        planes[0] = Plane(glm::vec4(
                vp[0][3] - vp[0][0],
                vp[1][3] - vp[1][0],
                vp[2][3] - vp[2][0],
                vp[3][3] - vp[3][0]));
        planes[1] = Plane(glm::vec4(
                vp[0][3] + vp[0][0],
                vp[1][3] + vp[1][0],
                vp[2][3] + vp[2][0],
                vp[3][3] + vp[3][0]));
        planes[2] = Plane(glm::vec4(
                vp[0][3] - vp[0][1],
                vp[1][3] - vp[1][1],
                vp[2][3] - vp[2][1],
                vp[3][3] - vp[3][1]));
        planes[3] = Plane(glm::vec4(
                vp[0][3] + vp[0][1],
                vp[1][3] + vp[1][1],
                vp[2][3] + vp[2][1],
                vp[3][3] + vp[3][1]));
        planes[4] = Plane(glm::vec4(
                vp[0][3] - vp[0][2],
                vp[1][3] - vp[1][2],
                vp[2][3] - vp[2][2],
                vp[3][3] - vp[3][2]));
        planes[5] = Plane(glm::vec4(
                vp[0][3] + vp[0][2],
                vp[1][3] + vp[1][2],
                vp[2][3] + vp[2][2],
                vp[3][3] + vp[3][2]));
    }

    bool contain(const BoundingSphere& bs) const {
        if (bs.empty())
            return false;
        for (const auto& plane : planes)
            if (plane.distanceTo(bs.center()) < -bs.radius()) return false;
        return true;
    }

    std::array<Plane, 6> planes;
};

#endif // FRUSTUM_H
