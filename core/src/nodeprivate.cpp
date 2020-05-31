#include <core/node.h>

#include "nodeprivate.h"
#include "sceneprivate.h"

#include "renderer.h"
#include "drawables.h"

#include <utils/frustum.h>

namespace trash
{
namespace core
{

const utils::BoundingBox NodePrivate::emptyLocalBoundingBox = utils::BoundingBox();

NodePrivate::NodePrivate(Node &node)
    : thisNode(node)
    , minimalBoundingBox()
    , isGlobalTransformDirty(true)
    , isBoundingBoxDirty(true)
{
}

NodePrivate::~NodePrivate()
{
}

void NodePrivate::dirtyGlobalTransform()
{
    isGlobalTransformDirty = true;
    for (auto child : thisNode.children())
        child->m().dirtyGlobalTransform();
}

void NodePrivate::dirtyBoundingBox()
{
    isBoundingBoxDirty = true;
    if (thisNode.parent())
        thisNode.parent()->m().dirtyBoundingBox();
}

void NodePrivate::doUpdate(uint64_t, uint64_t)
{
    auto& renderer = Renderer::instance();
    //renderer.draw(std::make_shared<BoxDrawable>(getBoundingBox(), glm::vec4(.0f, .8f, .0f, 1.0f)), getGlobalTransform());

    if (!thisNode.frustum)
        return;

    auto tr = getGlobalTransform().operator glm::mat4x4() *
            glm::translate(glm::mat4x4(1.0f), getBoundingBox().center()) *
            glm::rotate(glm::mat4x4(1.0f), glm::pi<float>(), glm::vec3(0.f, 1.f, 0.f));

    renderer.draw(std::make_shared<FrustumDrawable>(utils::Frustum(glm::perspective(glm::pi<float>()*0.25f, 1.0f, 100.0f, 3000.0f) * glm::inverse(tr)),
                                                    glm::vec4(1.f, 0.f, 0.f, 1.f)), utils::Transform());
}

Scene *NodePrivate::getScene() const
{
    Node *node = &thisNode;

    while (node->parent())
        node = node->parent();

    Scene *scene = nullptr;
    if (auto sceneRootNode = dynamic_cast<SceneRootNode*>(node))
        scene = sceneRootNode->scene();

    return scene;
}

const utils::BoundingBox &NodePrivate::getBoundingBox()
{
    if (isBoundingBoxDirty)
    {
        boundingBox = minimalBoundingBox;
        boundingBox += getLocalBoundingBox();
        for (auto child : thisNode.children())
            boundingBox += child->transform() * child->boundingBox();
        isBoundingBoxDirty = false;
    }
    return boundingBox;
}

const utils::Transform &NodePrivate::getGlobalTransform()
{
    if (isGlobalTransformDirty)
    {
        auto parent = thisNode.parent();
        globalTransform = parent ? parent->globalTransform() * transform : transform;

        isGlobalTransformDirty = false;
    }

    return globalTransform;
}

} // namespace
} // namespace
