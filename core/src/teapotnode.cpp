#include <glm/gtc/type_ptr.hpp>

#include <core/teapotnode.h>
#include <core/drawablenode.h>

#include "drawablenodeprivate.h"
#include "drawables.h"
#include "renderer.h"
#include "resources.h"

namespace trash
{
namespace core
{

TeapotNode::TeapotNode(const glm::vec3 &baseColor, float metallic, float roughness)
    : ModelNode(teapotModelName)
{
    auto& renderer = Renderer::instance();

    if (auto drawableNode = std::dynamic_pointer_cast<DrawableNode>(children().at(0)))
    {
        for (auto drawable : drawableNode->m().drawables)
        {
            auto texturedMeshDrawable = std::dynamic_pointer_cast<StandardDrawable>(drawable);
            if (texturedMeshDrawable)
            {
//                texturedMeshDrawable->baseColor = renderer.createTexture2D(GL_RGB16F, 1, 1, GL_RGB, GL_FLOAT, glm::value_ptr(baseColor));
//                texturedMeshDrawable->metallicOrSpecTexture = renderer.createTexture2D(GL_R16F, 1, 1, GL_RED, GL_FLOAT, &metallic);
//                texturedMeshDrawable->roughOrGlossTexture = renderer.createTexture2D(GL_R16F, 1, 1, GL_RED, GL_FLOAT, &roughness);
            }
        }
    }
}


} // namespace
} // namespace
