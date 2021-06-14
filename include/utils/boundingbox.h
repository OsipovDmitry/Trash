#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <vector>

#include "../glm/vec3.hpp"

#include "transform.h"
#include "boundingsphere.h"
#include "plane.h"

namespace trash
{
namespace utils
{

struct BoundingBox;
inline BoundingBox operator +(const BoundingBox&, const BoundingBox&);
inline BoundingBox operator +(const BoundingBox&, const BoundingSphere&);

struct BoundingBox
{
    glm::vec3 minPoint, maxPoint;

    BoundingBox() : minPoint(std::numeric_limits<float>::max()), maxPoint(-std::numeric_limits<float>::max()) {}
    BoundingBox(const glm::vec3& minP, const glm::vec3& maxP) : minPoint(minP), maxPoint(maxP) {}
    BoundingBox(float *p, uint32_t nv, uint32_t nc) : minPoint(std::numeric_limits<float>::max()), maxPoint(-std::numeric_limits<float>::max()) {
        const uint32_t stride = nc;
        nc = glm::min(nc, 3u);
        if (nv > 0) {
            minPoint = maxPoint = glm::vec3(0.0f);
            for (size_t i = 0; i < nc; ++i) minPoint[i] = maxPoint[i] = p[stride*0 + i];
        }
        for (size_t i = 1; i < nv; ++i)
            for (size_t k = 0; k < nc; ++k) {
                const size_t idx = i*stride+k;
                if (p[idx] < minPoint[k]) minPoint[k] = p[idx];
                else if (p[idx] > maxPoint[k]) maxPoint[k] = p[idx];
            }
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
        return (vMinDist * vMaxDist <= .0f) ?
                    .0f :
                    (vMinDist > .0f) ? vMinDist  : vMaxDist;
    }

    std::pair<float, float> pairDistancesToPlane(const Plane& p) const
    {
        glm::vec3 vmin = minPoint, vmax = maxPoint;
        for (size_t k = 0; k < 3; ++k)
            if (p[k] < .0f) std::swap(vmin[k], vmax[k]);
        const float vMinDist = p.distanceTo(vmin), vMaxDist = p.distanceTo(vmax);
        return std::make_pair(vMinDist, vMaxDist);
    }

    glm::vec3 closestPoint(const glm::vec3& v) const
    {
        return glm::clamp(v, minPoint, maxPoint);
    }

    glm::vec3 center() const { return .5f * (minPoint + maxPoint); }
    glm::vec3 halfSize() const { return .5f * (maxPoint - minPoint); }

    BoundingBox& operator += (const BoundingBox& b) { *this = *this + b; return *this; }
    BoundingBox& operator += (const BoundingSphere& s) { *this = *this + s; return *this; }

    static BoundingBox fromMinMax(const glm::vec3& minP, const glm::vec3& maxP) { return BoundingBox(minP, maxP); }
    static BoundingBox fromCenterHalfSize(const glm::vec3& cP, const glm::vec3& hz) { return BoundingBox(cP-hz, cP+hz); }
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

inline BoundingBox operator +(const BoundingBox& b1, const BoundingSphere& s2)
{
    return b1 + BoundingBox(s2.center() - glm::vec3(s2.radius()), s2.center() + glm::vec3(s2.radius()));
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
