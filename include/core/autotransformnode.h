#ifndef AUTOTRANSFORMNODE_H
#define AUTOTRANSFORMNODE_H

#include <core/node.h>

namespace trash
{
namespace core
{

class AutoTransformNodePrivate;

class CORESHARED_EXPORT AutoTransformNode : public Node
{
    PIMPL(AutoTransformNode)

public:
    AutoTransformNode();

};

} // namespace
} // namespace

#endif // AUTOTRANSFORMNODE_H
