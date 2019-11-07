#ifndef PLANE_H
#define PLANE_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/geometric.hpp>

class Plane : public glm::vec4
{
public:
    Plane() : glm::vec4(0.f, 1.f, 0.f, 0.f) {}
    Plane(const glm::vec3& n, float d) : glm::vec4(glm::normalize(n), -d) {}
    Plane(const glm::vec4& p) : glm::vec4(p) { *this /= glm::length(normal()); }

    glm::vec3 normal() const { return glm::vec3(x, y, z); }
    void setNormal(const glm::vec3& n) { glm::vec3 nn = glm::normalize(n); x = nn.x; y = nn.y; z = nn.z; }

    float dist() const { return -w; }
    void setDist(float r) { w = -r; }

    float distanceTo(const glm::vec3& p) const { return x * p.x + y * p.y + z * p.z + w; }

};

#endif // PLANE_H
