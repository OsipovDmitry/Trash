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

        static const std::vector<glm::vec4> clipPoints {
            glm::vec4(-1.f, -1.f, -1.f, 1.f),
            glm::vec4(-1.f,  1.f, -1.f, 1.f),
            glm::vec4( 1.f,  1.f, -1.f, 1.f),
            glm::vec4( 1.f, -1.f, -1.f, 1.f),
            glm::vec4(-1.f, -1.f,  1.f, 1.f),
            glm::vec4(-1.f,  1.f,  1.f, 1.f),
            glm::vec4( 1.f,  1.f,  1.f, 1.f),
            glm::vec4( 1.f, -1.f,  1.f, 1.f)
        };

        const auto vpInverse = glm::inverse(vp);
        vertices.resize(8);
        for (size_t i = 0; i < 8; ++i)
        {
            const glm::vec4 p = vpInverse * clipPoints[i];
            vertices[i] = glm::vec3(p) / p.w;
        }
    }

    float distanceToPlane(const Plane& p) const
    {
        float minDist = std::numeric_limits<float>::max(), maxDist = std::numeric_limits<float>::min();
        for (const auto& v : vertices)
        {
            float d = p.distanceTo(v);
            minDist = glm::min(minDist, d);
            maxDist = glm::max(maxDist, d);
        }
        return (minDist * maxDist <= .0f) ?
                    0.0f :
                    (minDist > .0f) ? minDist : maxDist;
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

    bool contain(const Frustum& f) const
    {
        for (const auto& plane : planes)
            if (f.distanceToPlane(plane) < .0f)
                return false;

        for (const auto& plane : f.planes)
            if (distanceToPlane(plane) < .0f)
                return false;

        return true;
    }

    std::vector<Plane> planes;
    std::vector<glm::vec3> vertices;
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
