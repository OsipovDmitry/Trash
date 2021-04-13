#include "textnodeprivate.h"
#include "drawables.h"
#include "renderer.h"

namespace trash
{
namespace core
{

TextNodePrivate::TextNodePrivate(Node& thisNode)
    : DrawableNodePrivate(thisNode)
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
        if (textDrawable)
            removeDrawable(textDrawable);
        textDrawable = std::make_shared<TextDrawable>(Renderer::instance().loadFont(":/res/PurisaDesc.json"), text, alignX, alignY, color, lineSpacing);
        addDrawable(textDrawable);

        drawableIsDyrty = false;
    }
}

void TextNodePrivate::doUpdate(uint64_t time, uint64_t dt)
{
    updateDrawable();
    DrawableNodePrivate::doUpdate(time, dt);
}


} // namespace
} // namespace
