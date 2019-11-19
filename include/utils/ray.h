#ifndef RAY_H
#define RAY_H

#include <glm/gtx/intersect.hpp>

#include "boundingsphere.h"

class Ray
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
};

#endif // RAY_H
