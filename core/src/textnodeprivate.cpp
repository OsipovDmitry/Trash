#include "textnodeprivate.h"
#include "drawables.h"
#include "renderer.h"

namespace trash
{
namespace core
{

TextNodePrivate::TextNodePrivate(Node& thisNode)
    : NodePrivate(thisNode)
    , drawableIsDyrty(true)
{
}

void TextNodePrivate::dirtyDrawable()
{
    drawableIsDyrty = true;
}

void TextNodePrivate::updateDrawable()
{
    if (drawableIsDyrty)
    {
        auto& renderer = Renderer::instance();
        drawable = std::make_shared<TextDrawable>(renderer.loadFont(":/res/PurisaDesc.json"), text, alignX, alignY, color, lineSpacing, localBoundingBox);
        drawableIsDyrty = false;
    }
}

const utils::BoundingBox &TextNodePrivate::getLocalBoundingBox()
{
    return localBoundingBox;
}

void TextNodePrivate::doUpdate(uint64_t dt, uint64_t time)
{
    NodePrivate::doUpdate(dt, time);

    updateDrawable();

    auto& renderer = Renderer::instance();
    renderer.draw(drawable, getGlobalTransform());
}


} // namespace
} // namespace
