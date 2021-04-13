#include <glm/gtc/quaternion.hpp>

#include <core/scene.h>
#include <core/node.h>

#include "autotransformnodeprivate.h"

namespace trash
{
namespace core
{

AutoTransformNodePrivate::AutoTransformNodePrivate(Node& node)
    : NodePrivate(node)
{

}

void AutoTransformNodePrivate::doUpdate(uint64_t time, uint64_t dt)
{
    NodePrivate::doUpdate(time, dt);

    auto scene = getScene();
    if (scene)
    {
        auto thisTransform = thisNode.transform();
        thisTransform.rotation = glm::inverse(glm::quat(glm::mat3x3(scene->viewMatrix())) * thisNode.parent()->globalTransform().rotation);
        thisNode.setTransform(thisTransform);
    }
}


} // namespace
} // namespace
