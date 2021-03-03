#ifndef DRAWABLENODEPRIVATE_H
#define DRAWABLENODEPRIVATE_H

#include <unordered_set>

#include "nodeprivate.h"
#include "typesprivate.h"

namespace trash
{
namespace core
{

class Drawable;
struct Texture;

class DrawableNodePrivate : public NodePrivate
{
public:
    DrawableNodePrivate(Node&);

    void addDrawable(std::shared_ptr<Drawable>);
    void removeDrawable(std::shared_ptr<Drawable>);
    void removeAllDrawables();
    void dirtyDrawables();

    const utils::BoundingBox& getLocalBoundingBox() override;
    std::shared_ptr<LightIndicesList> getLightIndices();

    virtual void doUpdateLightIndices();
    virtual void doDirtyLightIndices();
    virtual void doDirtyShadowMaps();

    virtual void doRender(uint32_t);

    void doUpdate(uint64_t, uint64_t) override;
    void doBeforeChangingTransformation() override;
    void doAfterChangingTransformation() override;

    std::unordered_set<std::shared_ptr<Drawable>> drawables;
    std::shared_ptr<LightIndicesList> lightIndices;
    utils::BoundingBox localBoundingBox;
    IntersectionMode intersectionMode;
    bool isLightIndicesDirty;
    bool isLocalBoundingBoxDirty;
    bool areShadowsEnabled;

};

} // namespace
} // namespace

#endif // DRAWABLENODEPRIVATE_H
