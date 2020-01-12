#ifndef SPHERE_H
#define SPHERE_H

#include <core/forwarddecl.h>
#include <core/node.h>

#include "object.h"

namespace trash
{
namespace game
{

class SphereNode : public core::Node
{

};

class Sphere : public Object
{
public:
    Sphere();

protected:
    std::shared_ptr<SphereNode> m_modelNode;

};

} // namespace
} // namespace

#endif // SPHERE_H
