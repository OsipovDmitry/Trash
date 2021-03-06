#ifndef NODEPRIVATE_H
#define NODEPRIVATE_H

#include <memory>
#include <array>

#include <glm/vec4.hpp>

#include <utils/noncopyble.h>
#include <utils/transform.h>
#include <utils/boundingbox.h>
#include <core/forwarddecl.h>

namespace trash
{
namespace core
{

class NodePrivate
{
public:
    NONCOPYBLE(NodePrivate)

    NodePrivate(Node& node);
    virtual ~NodePrivate();

    void dirtyGlobalTransform();
    void dirtyBoundingBox();

    virtual const utils::BoundingBox& getLocalBoundingBox() { return emptyLocalBoundingBox; }

    virtual void doUpdate(uint64_t, uint64_t);
    virtual void doBeforeChangingTransformation();
    virtual void doAfterChangingTransformation();

    Scene *getScene() const;
    const utils::BoundingBox& getBoundingBox();
    const utils::Transform& getGlobalTransform();

    Node& thisNode;
    utils::Transform transform, globalTransform;
    utils::BoundingBox minimalBoundingBox, boundingBox;

    std::shared_ptr<NodeUserData> userData;

    bool isGlobalTransformDirty;
    bool isBoundingBoxDirty;

    static const utils::BoundingBox emptyLocalBoundingBox;
};

} // namespace
} // namespace

#endif // NODEPRIVATE_H
