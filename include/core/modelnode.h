#ifndef MODELNODE_H
#define MODELNODE_H

#include <string>

#include "node.h"

namespace trash
{
namespace core
{

class ModelNodePrivate;

class CORESHARED_EXPORT ModelNode : public Node
{
    PIMPL(ModelNode)

public:
    ModelNode(const std::string&);

    bool isModelNode() const override;

    void showBones(bool);

//    void copyAnimation(std::shared_ptr<ModelNode>, const std::string&);
    void playAnimation(const std::string&, uint64_t timeOffset = 0);
    uint64_t animationTime(const std::string&) const;

};

} // namespace
} // namespace

#endif // MODELNODE_H
