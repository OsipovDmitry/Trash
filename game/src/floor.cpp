#include <core/modelnode.h>

#include "floor.h"

Floor::Floor()
    : Object(std::make_shared<ObjectUserData>(*this))
{
    m_modelNode = std::make_shared<ModelNode>("floor.mdl");
    m_graphicsNode->attach(m_modelNode);
}
