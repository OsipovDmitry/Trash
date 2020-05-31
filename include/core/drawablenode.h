#ifndef DRAWABLENODE_H
#define DRAWABLENODE_H

#include <utils/enumclass.h>

#include <core/node.h>

namespace trash
{
namespace core
{

ENUMCLASS(IntersectionMode, uint8_t, UseBoundingBox, UseGeometry)

class DrawableNodePrivate;

class CORESHARED_EXPORT DrawableNode : public Node
{
    PIMPL(DrawableNode)

public:
    DrawableNode();

    DrawableNode *asDrawableNode() override;
    const DrawableNode *asDrawableNode() const override;

    void setIntersectionMode(IntersectionMode);
    IntersectionMode intersectionMode() const;

protected:
    DrawableNode(NodePrivate*);

    void doAttach() override;
    void doDetach() override;

};

} // namespace
} // namespace

#endif // DRAWABLENODE_H
