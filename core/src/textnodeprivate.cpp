#include "textnodeprivate.h"
#include "drawables.h"
#include "renderer.h"
#include "utils.h"

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

        std::shared_ptr<Font> font = Renderer::instance().loadFont(":/res/PurisaDesc.json");
        textDrawable = std::make_shared<StandardDrawable>(buildTextMesh(font, text, alignX, alignY, lineSpacing), nullptr, color, glm::vec2(1.f, 1.f), nullptr, font->texture, nullptr, nullptr, nullptr, std::cref(getLightIndices()));
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
