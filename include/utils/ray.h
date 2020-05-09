#ifndef RAY_H
#define RAY_H

#include <glm/gtx/intersect.hpp>

#include "boundingsphere.h"
#include "boundingbox.h"

namespace trash
{
namespace utils
{

struct Ray
{
public:
    glm::vec3 pos, dir;

    Ray(const glm::vec3& p, const glm::vec3& d) : pos(p), dir(glm::normalize(d)) {}

    bool intersect(const BoundingSphere& bs) const {
        if (bs.empty())
            return false;

        glm::vec3 p, n;
        return glm::intersectRaySphere(pos, dir, bs.center(), bs.radius(), p, n);
    }

    bool intersect(const BoundingBox& bb) const {
        if (bb.empty())
            return false;

        float tmin = -FLT_MAX, tmax = FLT_MAX;

        for (size_t k = 0; k < 3; ++k) {
            float coordMin = (bb.minPoint[k] - pos[k]) / dir[k];
            float coordMax = (bb.maxPoint[k] - pos[k]) / dir[k];
            if (coordMin > coordMax) std::swap(coordMin, coordMax);
            if ((tmin > coordMax) || (tmax < coordMin)) return false;
            if (coordMin > tmin) tmin = coordMin;
            if (coordMax < tmax) tmax = coordMax;
        }

        return true;
    }
};

} // namespace
} // namespace

#endif // RAY_H
