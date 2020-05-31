#ifndef DRAWABLENODEPRIVATE_H
#define DRAWABLENODEPRIVATE_H

#include <unordered_set>

#include "nodeprivate.h"

namespace trash
{
namespace core
{

class Drawable;
struct Texture;
using ShadowMapsList = std::array<std::shared_ptr<Texture>, MAX_LIGHTS_PER_NODE>;

class DrawableNodePrivate : public NodePrivate
{
public:
    DrawableNodePrivate(Node&);

    void addDrawable(std::shared_ptr<Drawable>);
    void removeDrawable(std::shared_ptr<Drawable>);

    const utils::BoundingBox& getLocalBoundingBox() override;
    std::shared_ptr<LightIndicesList> getLightIndices();

    virtual void doUpdateLightIndices();
    virtual void doUpdateShadowMaps();
    virtual void doDirtyLightIndices();
    virtual void doDirtyShadowMaps();

    void doUpdate(uint64_t, uint64_t) override;
    void doPick(uint32_t) override;
    void doBeforeChangingTransformation() override;
    void doAfterChangingTransformation() override;

    std::unordered_set<std::shared_ptr<Drawable>> drawables;
    std::shared_ptr<LightIndicesList> lightIndices;
    utils::BoundingBox localBoundingBox;
    IntersectionMode intersectionMode;
    bool isLightIndicesDirty;
    bool isLocalBoundingBoxDirty;

};

} // namespace
} // namespace

#endif // DRAWABLENODEPRIVATE_H
