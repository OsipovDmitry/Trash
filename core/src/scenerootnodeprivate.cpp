#include <core/scene.h>
#include <core/light.h>

#include "scenerootnodeprivate.h"
#include "sceneprivate.h"

namespace trash
{
namespace core
{

SceneRootNodePrivate::SceneRootNodePrivate(Node& node, Scene* s)
    : NodePrivate(node)
    , scene(s)
    , localBoundingBox()
    , isLocalBoundingBoxDirty(true)
{
}

void SceneRootNodePrivate::dirtyLocalBoundingBox()
{
    isLocalBoundingBoxDirty = true;
    dirtyBoundingBox();
}

const utils::BoundingBox &SceneRootNodePrivate::getLocalBoundingBox()
{
    if (isLocalBoundingBoxDirty)
    {
        localBoundingBox = utils::BoundingBox();

        for (auto light: *scene->m().lights)
        {
            if (!light)
                continue;

            const auto lightType = light->type();
            if (lightType == LightType::Point || lightType == LightType::Spot)
            {
                const auto radiuses = light->radiuses();
                localBoundingBox += utils::BoundingSphere(light->position(), radiuses.x + radiuses.y);
            }
        }

        localBoundingBox = getGlobalTransform().inverted() * localBoundingBox;

        isLocalBoundingBoxDirty = false;
    }

    return localBoundingBox;
}

void SceneRootNodePrivate::doAfterChangingTransformation()
{
    dirtyBoundingBox();
}

} // namespace
} // namespace
