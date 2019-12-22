#ifndef NODEPRIVATE_H
#define NODEPRIVATE_H

#include <memory>
#include <unordered_set>
#include <array>

#include <glm/vec4.hpp>

#include <utils/transform.h>
#include <utils/boundingsphere.h>
#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

static const size_t MAX_LIGHTS_PER_NODE = 8;
using LightList = std::array<int32_t, MAX_LIGHTS_PER_NODE>;

class Drawable;

class NodePrivate
{
public:
    NodePrivate(Node& node);
    virtual ~NodePrivate();

    void dirtyGlobalTransform();
    void dirtyBoundingSphere();
    void dirtyLights();

    virtual void doUpdate(uint64_t, uint64_t);
    virtual void doPick(uint32_t);

    const utils::BoundingSphere& getBoundingSphere();
    const utils::Transform& getGlobalTransform();
    const LightList& getLights(const std::vector<std::shared_ptr<Light>>&);

    void addDrawable(std::shared_ptr<Drawable>);
    void removeDrawable(std::shared_ptr<Drawable>);

    Node& thisNode;
    std::unordered_set<std::shared_ptr<Drawable>> drawables;
    utils::Transform transform, globalTransform;
    utils::BoundingSphere minimalBoundingSphere, boundingSphere;
    LightList lights;

    std::shared_ptr<Drawable> bSphereDrawable;
    std::shared_ptr<NodeUserData> userData;

    bool isGlobalTransformDirty;
    bool isBoundingSphereDirty;
    bool isLightsDirty;
};

} // namespace
} // namespace

#endif // NODEPRIVATE_H
