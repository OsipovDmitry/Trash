#include <core/modelnode.h>

#include "floor.h"

namespace trash
{
namespace game
{

Floor::Floor()
    : Object(std::make_shared<ObjectUserData>(*this))
{
    m_modelNode = std::make_shared<core::ModelNode>("floor.mdl");
    m_graphicsNode->attach(m_modelNode);
}

} // namespace
} // namespace
