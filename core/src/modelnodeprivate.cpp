#include <core/core.h>
#include <core/node.h>

#include "coreprivate.h"
#include "modelnodeprivate.h"
#include "renderwidget.h"
#include "renderer.h"
#include "drawables.h"

ModelNodePrivate::ModelNodePrivate(Node& node)
    : NodePrivate(node)
    , timeOffset(0)
{
}

void ModelNodePrivate::doUpdate(uint64_t time, uint64_t dt)
{
    NodePrivate::doUpdate(time, dt);

    std::vector<Transform> bones;
    model->calcBoneTransforms(animationName, (time + timeOffset) * 0.001f, bones);

    void *pData = bonesBuffer->map(0, static_cast<GLsizeiptr>(bones.size()*sizeof(glm::mat3x4)), GL_MAP_WRITE_BIT);
    for (size_t i = 0; i < bones.size(); ++i)
        reinterpret_cast<glm::mat3x4*>(pData)[i] = glm::transpose(bones[i].operator glm::mat4x4());
    bonesBuffer->unmap();
}

