#include <core/textnode.h>

#include "textnodeprivate.h"

namespace trash
{
namespace core
{

TextNode::TextNode(const std::string& text_, TextNodeAlignment alignX, TextNodeAlignment alignY, const glm::vec4& color_, float lineSpacing_)
    : DrawableNode(new TextNodePrivate(*this))
{
    auto& tnPrivate = m();
    tnPrivate.text = text_;
    tnPrivate.alignX = alignX;
    tnPrivate.alignY = alignY;
    tnPrivate.color = color_;
    tnPrivate.lineSpacing = lineSpacing_;
    tnPrivate.dirtyDrawable();
    tnPrivate.updateDrawable(); // for calculating bounding box
}

const std::string &TextNode::text() const
{
    return m().text;
}

void TextNode::setText(const std::string& value)
{
    auto& tnPrivate = m();
    if (tnPrivate.text != value)
    {
        tnPrivate.text = value;
        tnPrivate.dirtyDrawable();
    }
}

const glm::vec4 &TextNode::color() const
{
    return m().color;
}

void TextNode::setColor(const glm::vec4& value)
{
    auto& tnPrivate = m();
    tnPrivate.color = value;
    tnPrivate.dirtyDrawable();
}

float TextNode::lineSpacing() const
{
    return m().lineSpacing;
}

void TextNode::setLineSpaceng(float value)
{
    auto& tnPrivate = m();
    tnPrivate.lineSpacing = value;
    tnPrivate.dirtyDrawable();
}

TextNodeAlignment TextNode::alignmentX() const
{
    return m().alignX;
}

TextNodeAlignment TextNode::alignmentY() const
{
    return m().alignY;
}

void TextNode::setAlignmentX(TextNodeAlignment value)
{
    auto& tnPrivate = m();
    tnPrivate.alignX = value;
    tnPrivate.dirtyDrawable();
}

void TextNode::setAlignmentY(TextNodeAlignment value)
{
    auto& tnPrivate = m();
    tnPrivate.alignY = value;
    tnPrivate.dirtyDrawable();
}

} // namespace
} // namespace
