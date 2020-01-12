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

struct UpdateInfo
{
    uint64_t time;
    uint64_t dt;
    const LightsList& lightsList;
};

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

    Scene *getScene() const;
    const utils::BoundingSphere& getBoundingSphere();
    const utils::Transform& getGlobalTransform();
    const LightIndicesList& getLights();

    void addDrawable(std::shared_ptr<Drawable>);
    void removeDrawable(std::shared_ptr<Drawable>);

    Node& thisNode;
    std::unordered_set<std::shared_ptr<Drawable>> drawables;
    utils::Transform transform, globalTransform;
    utils::BoundingSphere minimalBoundingSphere, boundingSphere;
    LightIndicesList lights;

    std::shared_ptr<Drawable> bSphereDrawable;
    std::shared_ptr<NodeUserData> userData;

    bool isGlobalTransformDirty;
    bool isBoundingSphereDirty;
    bool isLightsDirty;
};

} // namespace
} // namespace

#endif // NODEPRIVATE_H
