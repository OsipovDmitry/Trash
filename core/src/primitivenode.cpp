#include <core/primitivenode.h>

#include "drawablenodeprivate.h"
#include "utils.h"
#include "drawables.h"

namespace trash
{
namespace core
{

PrimitiveNode::PrimitiveNode()
    : DrawableNode()
{
    setIntersectionMode(IntersectionMode::None);

    auto& primitiveNodePrivate = m();
    primitiveNodePrivate.lightIndices.isEnabled = false;
    primitiveNodePrivate.areShadowsEnabled = false;
}

void PrimitiveNode::addLine(const glm::vec4& color, const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& colors, bool loop)
{
    auto& drawableNodePrivate = m();
    drawableNodePrivate.addDrawable(std::make_shared<StandardDrawable>(
                                        buildLineMesh(vertices, colors, loop),
                                        nullptr, color, glm::vec2(1.f,1.f), nullptr, nullptr, nullptr, nullptr, nullptr,
                                        drawableNodePrivate.getLightIndices())
                                    );
}

void PrimitiveNode::addSphere(const glm::vec4& color, uint32_t segs, const utils::BoundingSphere& bs, bool wf)
{
    auto& drawableNodePrivate = m();
    drawableNodePrivate.addDrawable(std::make_shared<StandardDrawable>(
                                        buildSphereMesh(segs, bs, wf),
                                        nullptr, color, glm::vec2(1.f,1.f), nullptr, nullptr, nullptr, nullptr, nullptr,
                                        drawableNodePrivate.getLightIndices())
                                    );
}

void PrimitiveNode::addBox(const glm::vec4& color, const utils::BoundingBox& bb, bool wf)
{
    auto& drawableNodePrivate = m();
    drawableNodePrivate.addDrawable(std::make_shared<StandardDrawable>(
                                        buildBoxMesh(bb, wf),
                                        nullptr, color, glm::vec2(1.f,1.f), nullptr, nullptr, nullptr, nullptr, nullptr,
                                        drawableNodePrivate.getLightIndices())
                                    );
}

void PrimitiveNode::addFrustum(const glm::vec4& color, const utils::Frustum& f)
{
    auto& drawableNodePrivate = m();
    drawableNodePrivate.addDrawable(std::make_shared<StandardDrawable>(
                                        buildFrustumMesh(f),
                                        nullptr, color, glm::vec2(1.f,1.f), nullptr, nullptr, nullptr, nullptr, nullptr,
                                        drawableNodePrivate.getLightIndices())
                                    );
}

void PrimitiveNode::addCone(const glm::vec4& color, uint32_t segs, float radius, float length, bool wf)
{
    auto& drawableNodePrivate = m();
    drawableNodePrivate.addDrawable(std::make_shared<StandardDrawable>(
                                        buildConeMesh(segs, radius, length, wf),
                                        nullptr, color, glm::vec2(1.f,1.f), nullptr, nullptr, nullptr, nullptr, nullptr,
                                        drawableNodePrivate.getLightIndices())
                                    );
}


} // namespace
} // namespace
