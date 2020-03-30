#ifndef TEXTNODEPRIVATE_H
#define TEXTNODEPRIVATE_H

#include <core/forwarddecl.h>

#include "nodeprivate.h"

namespace trash
{
namespace core
{

class TextDrawable;

class TextNodePrivate : public NodePrivate
{
public:
    TextNodePrivate(Node&);
    void dirtyDrawable();
    void updateDrawable();

    const utils::BoundingSphere& getLocalBoundingSphere() override;
    void doUpdate(uint64_t, uint64_t) override;

    utils::BoundingSphere localBoundingSphere;
    std::shared_ptr<TextDrawable> drawable;
    std::string text;
    glm::vec4 color;
    float lineSpacing;
    TextNodeAlignment alignX, alignY;
    bool drawableIsDyrty;
};

} // namespace
} // namespace

#endif // TEXTNODEPRIVATE_H
