#ifndef DRAWABLENODE_H
#define DRAWABLENODE_H

#include <core/node.h>

namespace trash
{
namespace core
{

class Drawable;
class DrawableNodePrivate;

class CORESHARED_EXPORT DrawableNode : public Node
{
    PIMPL(DrawableNode)

public:
    DrawableNode();

    bool isDrawableNode() const override;

    void addDrawable(std::shared_ptr<Drawable>);
    void removeDrawable(std::shared_ptr<Drawable>);

protected:
    DrawableNode(NodePrivate*);

};

} // namespace
} // namespace

#endif // DRAWABLENODE_H
