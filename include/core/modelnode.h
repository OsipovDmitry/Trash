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

    void showBones(bool);

//    void copyAnimation(std::shared_ptr<ModelNode>, const std::string&);
    void playAnimation(const std::string&, uint32_t timeOffset = 0);

};

#endif // MODELNODE_H
