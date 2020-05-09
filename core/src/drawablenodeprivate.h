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

    const utils::BoundingBox& getLocalBoundingBox() override;
    std::shared_ptr<LightIndicesList> getLightIndices();

    void doUpdate(uint64_t, uint64_t) override;
    void doPick(uint32_t) override;
    void doUpdateShadowMaps() override;
    void doDirtyLightIndices() override;
    void doDirtyShadowMaps() override;

    void updateLightIndices();

    std::unordered_set<std::shared_ptr<Drawable>> drawables;
    std::shared_ptr<LightIndicesList> lightIndices;
    utils::BoundingBox localBoundingBox;

    bool isLightIndicesDirty;
    bool isLocalBoundingBoxDirty;

};

} // namespace
} // namespace

#endif // DRAWABLENODEPRIVATE_H
