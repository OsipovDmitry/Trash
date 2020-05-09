#include <core/core.h>
#include <core/node.h>

#include "modelnodeprivate.h"
#include "sceneprivate.h"
#include "renderer.h"

namespace trash
{
namespace core
{

ModelNodePrivate::ModelNodePrivate(Node& node)
    : NodePrivate(node)
    , timeOffset(0)
    , timeStart(0)
    , showBones(false)
    , startAnimation(false)
{
}

void ModelNodePrivate::doUpdate(uint64_t time, uint64_t dt)
{
    NodePrivate::doUpdate(time, dt);

    if (model->numBones())
    {
        ScenePrivate::dirtyNodeShadowMaps(thisNode);

        if (startAnimation)
        {
            timeStart = time;
            startAnimation = false;
        }

        std::vector<glm::mat3x4> bones;
        model->calcBoneTransforms(animationName, (time - timeStart + timeOffset) * 0.001f, bones);
        bonesBuffer->setSubData(0, static_cast<GLsizeiptr>(bones.size()*sizeof(glm::mat3x4)), bones.data());
    }
}

} // namespace
} // namespace
