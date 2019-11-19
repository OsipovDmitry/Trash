#include <queue>

#include <core/core.h>
#include <core/node.h>

#include "coreprivate.h"
#include "modelnodeprivate.h"
#include "renderwidget.h"
#include "renderer.h"
#include "drawables.h"

ModelNodePrivate::ModelNodePrivate(Node &node, const std::string& modelName_)
    : NodePrivate(node)
    , modelName(modelName_)
    , timeOffset(0)
{
    auto& renderer = Renderer::instance();

    model = renderer.loadModel(modelName);
    animatedAttributesProgram = renderer.loadRenderProgram(":/res/skeletal_animation.vert", ":/res/skeletal_animation.frag");
    renderProgram = renderer.loadRenderProgram(":/res/shader.vert", ":/res/shader.frag");

    if (model->numBones())
        bonesBuffer = std::make_shared<Buffer>(model->numBones()*48, nullptr, GL_DYNAMIC_DRAW);

    std::queue<std::pair<std::shared_ptr<Model::Node>, Transform>> nodes;
    nodes.push(std::make_pair(model->rootNode, Transform()));

    while (!nodes.empty())
    {
        auto& nodeData = nodes.front();
        std::shared_ptr<Model::Node> node = nodeData.first;
        Transform transform = nodeData.second * node->transform;
        nodes.pop();

        for (auto mesh : node->meshes)
        {
            std::shared_ptr<VertexBuffer> animatedAttributesBuffer;
            if (mesh->hasAnimatedAttributes)
            {
                auto& vertexBuffers = mesh->mesh->vertexBuffers;
                if (!vertexBuffers.empty())
                {
                    const GLsizei numVertices = (*vertexBuffers.begin())->numVertices;
                    animatedAttributesBuffer = std::make_shared<VertexBuffer>(numVertices, numAttributeComponents(VertexAttribute::Position), nullptr, GL_DYNAMIC_DRAW);
                    animatedAttributesBuffer->declareAttribute(VertexAttribute::Position, 0);
                }
            }

            meshDrawables.push_back(std::make_pair(renderer.createMeshDrawable(renderProgram, mesh, animatedAttributesBuffer), transform));
            animatedAttributes.push_back(animatedAttributesBuffer);
        }

        for (auto child : node->children())
            nodes.push(std::make_pair(child, transform));
    }
}

void ModelNodePrivate::doUpdate(uint64_t time, uint64_t dt)
{
    NodePrivate::doUpdate(time, dt);

    std::vector<Transform> bones;
    model->calcBoneTransforms(animationName, (time + timeOffset) * 0.001f, bones);

    void *pData = bonesBuffer->map(0, static_cast<GLsizeiptr>(bones.size()*48), GL_MAP_WRITE_BIT);
    for (size_t i = 0; i < bones.size(); ++i)
        reinterpret_cast<glm::mat3x4*>(pData)[i] = glm::transpose(bones[i].operator glm::mat4x4());
    bonesBuffer->unmap();

    auto& renderer = Renderer::instance();

    for (size_t i = 0; i < meshDrawables.size(); ++i)
    {
        auto& drawable = meshDrawables.at(i);
        auto animatedAttributesBuffer = animatedAttributes.at(i);

        if (animatedAttributesBuffer)
        {
            renderer.bindTransformFeedbackBuffer(animatedAttributesBuffer, 0);
            renderer.bindUniformBuffer(bonesBuffer, animatedAttributesProgram->uniformBufferIndexByName("u_bonesBuffer"));

            renderer.beginTransformFeedback(GL_TRIANGLES);
            renderer.drawMesh(drawable.first->mesh(), animatedAttributesProgram);
            renderer.endTransformFeedback();
        }

        renderer.draw(0, drawable.first, thisNode.globalTransform() * drawable.second);
    }
}

BoundingSphere ModelNodePrivate::calcLocalBoundingSphere()
{
    return model->calcBoundingSphere();
}
