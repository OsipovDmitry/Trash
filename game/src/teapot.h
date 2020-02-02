#ifndef TEAPOT_H
#define TEAPOT_H

#include <glm/vec3.hpp>

#include <core/forwarddecl.h>

#include "object.h"

namespace trash
{
namespace game
{

class Teapot : public Object
{
public:
    Teapot(const glm::vec3& baseColor, float metallic, float roughness);

protected:
    std::shared_ptr<core::TeapotNode> m_teapotNode;

};

} // namespace
} // namespace

#endif // TEAPOT_H
