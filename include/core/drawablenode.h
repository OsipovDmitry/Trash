#ifndef DRAWABLENODE_H
#define DRAWABLENODE_H

#include <utils/enumclass.h>

#include <core/node.h>

namespace trash
{
namespace core
{

ENUMCLASS(IntersectionMode, uint8_t, None, UseBoundingBox, UseGeometry)

class DrawableNodePrivate;

class CORESHARED_EXPORT DrawableNode : public Node
{
    PIMPL(DrawableNode)

public:
    DrawableNode();

    void removeAllDrawables();

    void setIntersectionMode(IntersectionMode);
    IntersectionMode intersectionMode() const;

    void enableLighting(bool);
    bool isLightingEnabled() const;

    void enableShadows(bool);
    bool areShadowsEnabled() const;

protected:
    DrawableNode(NodePrivate*);

    void doAttach() override;
    void doDetach() override;

};

} // namespace
} // namespace

#endif // DRAWABLENODE_H
