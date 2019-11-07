#ifndef BOUNDINGSPHERE_H
#define BOUNDINGSPHERE_H

#include <vector>

#include "../glm/vec4.hpp"
#include "../glm/gtc/vec1.hpp"

#include "transform.h"

struct BoundingSphere;
inline BoundingSphere operator +(const BoundingSphere& s1, const BoundingSphere& s2);

struct BoundingSphere : public glm::vec4
{
    BoundingSphere() : glm::vec4(0.f, 0.f, 0.f, -1.f) {}
    BoundingSphere(const glm::vec3& c, float r) : glm::vec4(c, r) {}

    glm::vec3 center() const { return glm::vec3(x, y, z); }
    void setCenter(const glm::vec3& c) { x = c.x; y = c.y; z = c.z; }

    float radius() const { return w; }
    void setRadius(float r) { w = r; }

    bool empty() const { return w < .0f; }

    static bool contain(const BoundingSphere& first, const BoundingSphere& second) {
        if (second.empty()) return true;
        if (first.empty()) return false;
        return glm::distance(first.center(), second.center())  <= first.radius() - second.radius();
    }
    bool contain(const BoundingSphere& second) const { return contain(*this, second); }

    BoundingSphere& operator += (const BoundingSphere& s) { *this = *this + s; return *this; }

    static BoundingSphere unite(const std::vector<BoundingSphere>& spheres) {
        glm::vec3 c(0.f, 0.f, 0.f);
        size_t numNonNullSpheres = 0;
        for (const auto& bs : spheres)
            if (!bs.empty()) { c += bs.center(); ++numNonNullSpheres; }
        if (numNonNullSpheres)  c /= static_cast<float>(numNonNullSpheres);
        float r = -1.0f;
        for (const auto& bs : spheres)
            if (!bs.empty()) r = glm::max(r, glm::distance(c, bs.center()) + bs.radius());
        return BoundingSphere(c, r);
    }
};

inline BoundingSphere operator +(const BoundingSphere& s1, const BoundingSphere& s2)
{
    if (s2.contain(s1))
        return s2;

    if (s1.contain(s2))
        return s1;

    glm::vec3 dir = glm::normalize(s1.center() - s2.center());
    glm::vec3 p1 = s1.center() + s1.radius() * dir;
    glm::vec3 p2 = s2.center() - s2.radius() * dir;

    return BoundingSphere((p1 + p2) * .5f, glm::distance(p1, p2) * .5f);
}

inline BoundingSphere operator *(const Transform& t, const BoundingSphere& s)
{
    glm::vec3 newRadius = s.radius() < 0.f ? glm::vec3(s.radius()) : t.scale * s.radius();
    return BoundingSphere(t * s.center(), glm::max(glm::max(newRadius.x, newRadius.y), newRadius.z));
}

#endif // BOUNDINGSPHERE_H
