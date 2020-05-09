#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <vector>

#include <glm/mat4x4.hpp>

#include "plane.h"
#include "boundingsphere.h"
#include "boundingbox.h"

namespace trash
{
namespace utils
{

struct Frustum
{
public:
    Frustum(const glm::mat4x4& vp)
    {
        planes.resize(6);
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
                vp[0][3] + vp[0][2],
                vp[1][3] + vp[1][2],
                vp[2][3] + vp[2][2],
                vp[3][3] + vp[3][2]));
        planes[5] = Plane(glm::vec4(
                vp[0][3] - vp[0][2],
                vp[1][3] - vp[1][2],
                vp[2][3] - vp[2][2],
                vp[3][3] - vp[3][2]));

    }

    bool contain(const BoundingSphere& bs) const {
        if (bs.empty())
            return false;
        for (const auto& plane : planes)
            if (plane.distanceTo(bs.center()) < -bs.radius()) return false;
        return true;
    }

    bool contain(const BoundingBox& bb) const {
        if (bb.empty())
            return false;
        for (const auto& plane : planes)
            if (bb.distanceToPlane(plane) < .0f)
                return false;
        return true;
    }

    std::vector<Plane> planes;
};

class OpenFrustum : public Frustum
{
public:
    OpenFrustum(const glm::mat4x4& vp)
        : Frustum(vp)
    {
        planes.resize(5);
    }
};

} // namespace
} // namespace

#endif // FRUSTUM_H
