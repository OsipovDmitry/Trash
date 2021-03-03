#ifndef RESOURCES_H
#define RESOURCES_H

#include <string>

#include <glm/vec3.hpp>

namespace trash
{
namespace core
{

const std::pair<std::string, std::string> backgroundRenderProgramName { ":/res/background.vert", ":/res/background.frag" };
const std::pair<std::string, std::string> standardRenderProgramName { ":/res/standard.vert", ":/res/standard.frag" };
const std::pair<std::string, std::string> shadowRenderProgramName { ":/res/shadow.vert", ":/res/shadow.frag" };
const std::pair<std::string, std::string> idRenderProgramName { ":/res/id.vert", ":/res/id.frag" };
const std::pair<std::string, std::string> deferredGeometryPassRenderProgramName { ":/res/deferred_geometry_pass.vert", ":/res/deferred_geometry_pass.frag" };
const std::pair<std::string, std::string> deferredStencilPassRenderProgramName { ":/res/deferred_stencil_pass.vert", ":/res/deferred_stencil_pass.frag" };
const std::pair<std::string, std::string> deferredLightPassRenderProgramName { ":/res/deferred_light_pass.vert", ":/res/deferred_light_pass.frag" };
const std::pair<std::string, std::string> postEffectRenderProgramName { ":/res/posteffect.vert", ":/res/posteffect_final.frag" };

const std::string teapotModelName(":/res/teapot.fbx");
const std::string standardDiffuseTextureName(":/res/chess.png");

} // namespace
} // namespace

#endif // RESOURCES_H
