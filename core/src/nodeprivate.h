#ifndef NODEPRIVATE_H
#define NODEPRIVATE_H

#include <memory>

#include <utils/transform.h>
#include <core/forwarddecl.h>

class Renderer;

class NodePrivate
{
public:
    NodePrivate(Node& node);
    virtual ~NodePrivate();

    void dirtyGlobalTransform();

    virtual void doUpdate(Renderer&, uint64_t, uint64_t) {}

    Node& thisNode;
    Transform transform;
    Transform globalTransform;

    bool isGlobalTransformDirty;
};

#endif // NODEPRIVATE_H
