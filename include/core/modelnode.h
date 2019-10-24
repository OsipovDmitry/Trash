#ifndef MODELNODE_H
#define MODELNODE_H

#include <string>

#include "node.h"

class ModelNodePrivate;

class CORESHARED_EXPORT ModelNode : public Node
{
    PIMPL(ModelNode)

public:
    ModelNode(const std::string&);
};

#endif // MODELNODE_H
