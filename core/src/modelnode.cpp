#include <core/modelnode.h>

#include "modelnodeprivate.h"

ModelNode::ModelNode(const std::string &filename)
    : Node(new ModelNodePrivate(*this, filename))
{
}
