#ifndef NODEPRIVATE_H
#define NODEPRIVATE_H

#include <memory>
#include <unordered_set>

#include <utils/transform.h>
#include <utils/boundingsphere.h>
#include <core/forwarddecl.h>

class Drawable;

class NodePrivate
{
public:
    NodePrivate(Node& node);
    virtual ~NodePrivate();

    void dirtyGlobalTransform();
    void dirtyBoundingSphere();

    virtual void doUpdate(uint64_t, uint64_t);
    virtual void doPick(uint32_t);

    virtual const BoundingSphere& getBoundingSphere();

    void addDrawable(std::shared_ptr<Drawable>);
    void removeDrawable(std::shared_ptr<Drawable>);

    Node& thisNode;
    std::unordered_set<std::shared_ptr<Drawable>> drawables;
    Transform transform, globalTransform;
    BoundingSphere minimalBoundingSphere, boundingSphere;

    std::shared_ptr<Drawable> bSphereDrawable;
    std::shared_ptr<NodeUserData> userData;

    bool isGlobalTransformDirty;
    bool isBoundingSphereDirty;
};

#endif // NODEPRIVATE_H
