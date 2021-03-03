#ifndef PRIMITIVENODE_H
#define PRIMITIVENODE_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <utils/forwarddecl.h>
#include <core/drawablenode.h>

namespace trash
{
namespace core
{

class CORESHARED_EXPORT PrimitiveNode : public DrawableNode
{

public:
    PrimitiveNode();

    void addLine(const glm::vec4&, const std::vector<glm::vec3>&, const std::vector<glm::vec3>&, bool);
    void addSphere(const glm::vec4&, uint32_t, const utils::BoundingSphere&, bool);
    void addBox(const glm::vec4&, const utils::BoundingBox&, bool);
    void addFrustum(const glm::vec4&, const utils::Frustum&);
    void addCone(const glm::vec4&, uint32_t, float, float, bool);

};

} // namespace
} // namespace

#endif // PRIMITIVENODE_H
