#ifndef TEAPOTNODE_H
#define TEAPOTNODE_H

#include <glm/vec3.hpp>

#include <core/modelnode.h>

namespace trash
{
namespace core
{

class CORESHARED_EXPORT TeapotNode : public ModelNode
{
    PIMPL(ModelNode)

public:
    TeapotNode(const glm::vec3& baseColor, float metallic, float roughness);
};

} // namespace
} // namespace

#endif // TEAPOTNODE_H
