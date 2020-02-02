#include <core/teapotnode.h>

#include "teapot.h"

namespace trash
{
namespace game
{

Teapot::Teapot(const glm::vec3& baseColor, float metallic, float roughness)
    : Object(std::make_shared<ObjectUserData>(*this))
{
    m_teapotNode = std::make_shared<core::TeapotNode>(baseColor, metallic, roughness);
    m_graphicsNode->attach(m_teapotNode);
}

} // namespace
} // namespace
