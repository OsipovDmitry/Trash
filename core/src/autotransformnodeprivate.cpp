#include <glm/gtc/quaternion.hpp>

#include <core/scene.h>
#include <core/camera.h>

#include "autotransformnodeprivate.h"

namespace trash
{
namespace core
{

AutoTransformNodePrivate::AutoTransformNodePrivate(Node& node)
    : NodePrivate(node)
{

}

void AutoTransformNodePrivate::doUpdate(uint64_t, uint64_t)
{
    auto scene = getScene();
    if (scene)
    {
        dirtyShadowMaps(); // before transformation
        transform.rotation = glm::inverse(glm::quat(glm::mat3x3(scene->camera()->viewMatrix())) * thisNode.parent()->globalTransform().rotation);
        dirtyGlobalTransform();
        dirtyLightIndices();
        dirtyShadowMaps(); // after transformation
    }
}


} // namespace
} // namespace
