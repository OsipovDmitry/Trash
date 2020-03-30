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
        drawable = std::make_shared<TextDrawable>(renderer.loadFont(":/res/PurisaDesc.json"), text, alignX, alignY, color, lineSpacing, localBoundingSphere);
        drawableIsDyrty = false;
    }
}

const utils::BoundingSphere &TextNodePrivate::getLocalBoundingSphere()
{
    return localBoundingSphere;
}

void TextNodePrivate::doUpdate(uint64_t, uint64_t)
{
    updateDrawable();

    auto& renderer = Renderer::instance();
    renderer.draw(drawable, getGlobalTransform());
}


} // namespace
} // namespace
