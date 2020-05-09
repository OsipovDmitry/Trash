#ifndef DRAWABLENODE_H
#define DRAWABLENODE_H

#include <core/node.h>

namespace trash
{
namespace core
{

class DrawableNodePrivate;

class CORESHARED_EXPORT DrawableNode : public Node
{
    PIMPL(DrawableNode)

public:
    DrawableNode();

    DrawableNode *asDrawableNode() override;
    const DrawableNode *asDrawableNode() const override;

protected:
    DrawableNode(NodePrivate*);

    void doAttach() override;
    void doDetach() override;

};

} // namespace
} // namespace

#endif // DRAWABLENODE_H
