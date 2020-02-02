#include <core/teapotnode.h>

#include "modelnodeprivate.h"
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

    for (auto drawable : children().at(0)->m().drawables)
    {
        auto texturedMeshDrawable = std::dynamic_pointer_cast<TexturedMeshDrawable>(drawable);
        if (texturedMeshDrawable)
        {
            texturedMeshDrawable->baseColorTexture = renderer.loadTexture(baseColor);
            texturedMeshDrawable->metallicOrSpecTexture = renderer.loadTexture(metallic);
            texturedMeshDrawable->roughOrGlossTexture = renderer.loadTexture(roughness);
        }
    }
}


} // namespace
} // namespace
