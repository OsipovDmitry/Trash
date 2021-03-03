#ifndef RAY_H
#define RAY_H

#include <set>

#include <glm/gtx/intersect.hpp>

#include "transform.h"
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

    glm::vec3 calculatePoint(float t) const { return pos + t * dir; }

    bool intersect(const BoundingSphere& bs) const {
        if (bs.empty())
            return false;

        glm::vec3 p, n;
        return glm::intersectRaySphere(pos, dir, bs.center(), bs.radius(), p, n);
    }

    bool intersect(const BoundingBox& bb, float *t0 = nullptr, float *t1 = nullptr) const {
        if (bb.empty())
            return false;

        float tmin = -std::numeric_limits<float>::max(), tmax = std::numeric_limits<float>::max();

        for (size_t k = 0; k < 3; ++k) {
            float coordMin = (bb.minPoint[k] - pos[k]) / dir[k];
            float coordMax = (bb.maxPoint[k] - pos[k]) / dir[k];
            if (coordMin > coordMax) std::swap(coordMin, coordMax);
            if ((tmin > coordMax) || (tmax < coordMin)) return false;
            if (coordMin > tmin) tmin = coordMin;
            if (coordMax < tmax) tmax = coordMax;
        }

        if (t0) *t0 = tmin;
        if (t1) *t1 = tmax;
        return tmax > 0.f;
    }

    bool intersect(const glm::vec3* const vertices, const uint32_t* const indices, size_t numIndices, std::set<float>* ts)
    {
        bool result = false;
        glm::vec2 barycentric;
        float t = -1.f;

        for (size_t i = 0; i < numIndices; i += 3)
            if (glm::intersectRayTriangle(pos, dir, vertices[indices[i]], vertices[indices[i+1]], vertices[indices[i+2]], barycentric, t) && (t >= 0.f))
            {
                result = true;
                if (ts)
                    ts->insert(t);
            }

        return result;
    }

    bool intersect(const glm::vec2* const vertices, const uint32_t* const indices, size_t numIndices, std::set<float>* ts)
    {
        bool result = false;
        glm::vec2 barycentric;
        float t = -1.f;

        for (size_t i = 0; i < numIndices; i += 3)
            if (glm::intersectRayTriangle(pos, dir, glm::vec3(vertices[indices[i]],0.f), glm::vec3(vertices[indices[i+1]],0.f), glm::vec3(vertices[indices[i+2]],0.f), barycentric, t)
                    && (t >= 0.f))
            {
                result = true;
                if (ts)
                    ts->insert(t);
            }

        return result;
    }
};

inline Ray operator *(const Transform& t, const Ray& r)
{
    return Ray(t * r.pos, glm::vec3(t.rotation * glm::vec4(glm::normalize(t.scale * r.dir), 1.f)));
}

} // namespace
} // namespace

#endif // RAY_H
