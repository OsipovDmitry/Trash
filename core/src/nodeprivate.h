#ifndef NODEPRIVATE_H
#define NODEPRIVATE_H

#include <memory>

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
    std::shared_ptr<Drawable> boundingSphereDrawable();

    virtual void doUpdate(uint64_t, uint64_t);
    virtual BoundingSphere calcLocalBoundingSphere() { return BoundingSphere(); }

    Node& thisNode;
    Transform transform;
    Transform globalTransform;
    BoundingSphere boundingSphere;

    std::shared_ptr<Drawable> bSphereDrawable;

    bool isGlobalTransformDirty;
    bool isBoundingSphereDirty;
};

#endif // NODEPRIVATE_H
