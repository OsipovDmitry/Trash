#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <vector>
#include <cfloat>

#include "../glm/vec3.hpp"

#include "transform.h"
#include "plane.h"

namespace trash
{
namespace utils
{

struct BoundingBox;
inline BoundingBox operator +(const BoundingBox& s1, const BoundingBox& s2);

struct BoundingBox
{
    glm::vec3 minPoint, maxPoint;

    BoundingBox() : minPoint(FLT_MAX, FLT_MAX, FLT_MAX), maxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX) {}
    BoundingBox(const glm::vec3& minP, const glm::vec3& maxP) : minPoint(minP), maxPoint(maxP) {}
    BoundingBox(glm::vec3 *p, size_t nv) : minPoint(FLT_MAX, FLT_MAX, FLT_MAX), maxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX) {
        if (nv > 0) minPoint = maxPoint = p[0];
        for (size_t i = 1; i < nv; ++i)
            for (size_t k = 0; k < 3; ++k)
                if (p[i][k] < minPoint[k]) minPoint[k] = p[i][k];
                else if (p[i][k] > maxPoint[k]) maxPoint[k] = p[i][k];
    }
    BoundingBox(glm::vec2 *p, size_t nv) : minPoint(FLT_MAX, FLT_MAX, FLT_MAX), maxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX) {
        if (nv > 0) minPoint = maxPoint = glm::vec3(p[0], 0.0f);
        for (size_t i = 1; i < nv; ++i)
            for (size_t k = 0; k < 2; ++k)
                if (p[i][k] < minPoint[k]) minPoint[k] = p[i][k];
                else if (p[i][k] > maxPoint[k]) maxPoint[k] = p[i][k];
    }

    bool empty() const {
        for (size_t k = 0; k < 3; ++k)
            if (minPoint[k] > maxPoint[k]) return true;
        return false;
    }

    float distanceToPlane(const Plane& p) const {
        glm::vec3 vmin = minPoint, vmax = maxPoint;
        for (size_t k = 0; k < 3; ++k)
            if (p[k] < .0f) std::swap(vmin[k], vmax[k]);
        const float vMinDist = p.distanceTo(vmin), vMaxDist = p.distanceTo(vmax);
        return (vMinDist * vMaxDist < .0f) ?
                    .0f :
                    (vMinDist > .0f) ? vMinDist  : vMaxDist;
    }

    void distanceToPlane(const Plane& p, std::pair<float, float>& dists) const
    {
        glm::vec3 vmin = minPoint, vmax = maxPoint;
        for (size_t k = 0; k < 3; ++k)
            if (p[k] < .0f) std::swap(vmin[k], vmax[k]);
        const float vMinDist = p.distanceTo(vmin), vMaxDist = p.distanceTo(vmax);
        dists = std::make_pair(vMinDist, vMaxDist);
    }

    glm::vec3 center() const { return .5f * (minPoint + maxPoint); }
    glm::vec3 halfSize() const { return .5f * (maxPoint - minPoint); }

    BoundingBox& operator += (const BoundingBox& b) { *this = *this + b; return *this; }
};

inline BoundingBox operator +(const BoundingBox& b1, const BoundingBox& b2)
{
    BoundingBox result;;
    for (size_t k = 0; k < 3; ++k) {
        result.minPoint[k] = (b1.minPoint[k] < b2.minPoint[k]) ? b1.minPoint[k] : b2.minPoint[k];
        result.maxPoint[k] = (b1.maxPoint[k] > b2.maxPoint[k]) ? b1.maxPoint[k] : b2.maxPoint[k];
    }
    return result;
}

inline BoundingBox operator *(const Transform& t, const BoundingBox& b)
{
    if (b.empty()) return BoundingBox();

    const auto xAxis = glm::abs(glm::vec3(t.rotation * glm::vec4(1.f, 0.f, 0.f, 1.f)));
    const auto yAxis = glm::abs(glm::vec3(t.rotation * glm::vec4(0.f, 1.f, 0.f, 1.f)));
    const auto zAxis = glm::abs(glm::vec3(t.rotation * glm::vec4(0.f, 0.f, 1.f, 1.f)));

    const glm::vec3 halfSize = glm::mat3x3(xAxis, yAxis, zAxis) * (b.halfSize() * t.scale);
    const glm::vec3 center = t * b.center();

    return BoundingBox(center - halfSize, center + halfSize);
}

} // namespace
} // namespace

#endif // BOUNDINGBOX_H
