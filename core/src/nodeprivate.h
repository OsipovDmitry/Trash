#ifndef NODEPRIVATE_H
#define NODEPRIVATE_H

#include <memory>
#include <array>

#include <glm/vec4.hpp>

#include <utils/transform.h>
#include <utils/boundingsphere.h>
#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

class Drawable;

class NodePrivate
{
public:
    NodePrivate(Node& node);
    virtual ~NodePrivate();

    void dirtyGlobalTransform();
    void dirtyBoundingSphere();
    void dirtyLightIndices();
    void dirtyShadowMaps();

    virtual utils::BoundingSphere getLocalBoundingSphere() { return utils::BoundingSphere(); }

    virtual void doUpdate(uint64_t, uint64_t) {}
    virtual void doPick(uint32_t) {}
    virtual void doUpdateShadowMaps() {}
    virtual void doDirtyLightIndices() {}
    virtual void doDirtyShadowMaps() {}

    Scene *getScene() const;
    const utils::BoundingSphere& getBoundingSphere();
    const utils::Transform& getGlobalTransform();

    Node& thisNode;
    utils::Transform transform, globalTransform;
    utils::BoundingSphere minimalBoundingSphere, boundingSphere;

    std::shared_ptr<Drawable> bSphereDrawable;
    std::shared_ptr<NodeUserData> userData;

    bool isGlobalTransformDirty;
    bool isBoundingSphereDirty;
};

} // namespace
} // namespace

#endif // NODEPRIVATE_H
