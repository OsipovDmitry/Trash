#ifndef FLOOR_H
#define FLOOR_H

#include <core/forwarddecl.h>

#include "object.h"

namespace trash
{
namespace game
{

class Floor : public Object
{
public:
    Floor();

protected:
    std::shared_ptr<core::ModelNode> m_modelNode;

};

} // namespace
} // namespace

#endif // FLOOR_H
