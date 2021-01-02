#ifndef TEXTNODEPRIVATE_H
#define TEXTNODEPRIVATE_H

#include <core/forwarddecl.h>

#include "drawablenodeprivate.h"

namespace trash
{
namespace core
{

class TextNodePrivate : public DrawableNodePrivate
{
public:
    TextNodePrivate(Node&);
    void dirtyDrawable();
    void updateDrawable();

    void doUpdate(uint64_t, uint64_t) override;

    // it's unnecessary to update lighting and shading for textnode
    void doUpdateLightIndices() override {}
    void doDirtyLightIndices() override {}
    void doDirtyShadowMaps() override {}

    std::shared_ptr<Drawable> textDrawable;
    std::string text;
    glm::vec4 color;
    float lineSpacing;
    TextNodeAlignment alignX, alignY;
    bool drawableIsDyrty;
};

} // namespace
} // namespace

#endif // TEXTNODEPRIVATE_H
