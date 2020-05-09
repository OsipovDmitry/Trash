#ifndef TEXTNODE_H
#define TEXTNODE_H

#include <glm/vec4.hpp>

#include <utils/enumclass.h>
#include <core/drawablenode.h>

namespace trash
{
namespace core
{

ENUMCLASS(TextNodeAlignment, uint8_t, Negative, Center, Positive)

class TextNodePrivate;

class CORESHARED_EXPORT TextNode : public DrawableNode
{
    PIMPL(TextNode)

public:
    TextNode(const std::string&, TextNodeAlignment = TextNodeAlignment::Negative, TextNodeAlignment = TextNodeAlignment::Positive, const glm::vec4& = glm::vec4(1.f,1.f,1.f,1.f), float = 1.2f);

    const std::string& text() const;
    void setText(const std::string&);

    const glm::vec4& color() const;
    void setColor(const glm::vec4&);

    float lineSpacing() const;
    void setLineSpaceng(float);

    TextNodeAlignment alignmentX() const;
    TextNodeAlignment alignmentY() const;
    void setAlignmentX(TextNodeAlignment);
    void setAlignmentY(TextNodeAlignment);

};

} // namespace
} // namespace

#endif // TEXTNODE_H
