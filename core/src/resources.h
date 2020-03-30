#ifndef RESOURCES_H
#define RESOURCES_H

#include <string>

#include <glm/vec3.hpp>

namespace trash
{
namespace core
{

const std::pair<std::string, std::string> backgroundRenderProgramName { ":/res/background.vert", ":/res/background.frag" };

const std::pair<std::string, std::string> textRenderProgramName { ":/res/text.vert", ":/res/text.frag" };

const std::pair<std::string, std::string> coloreredMeshRenderProgramName { ":/res/colored_mesh.vert", ":/res/colored_mesh.frag" };
const std::pair<std::string, std::string> coloreredStaticMeshRenderProgramName { ":/res/colored_static_mesh.vert", ":/res/colored_mesh.frag" };

const std::pair<std::string, std::string> texturedMeshRenderProgramName { ":/res/textured_mesh.vert", ":/res/textured_mesh.frag" };
const std::pair<std::string, std::string> texturedStaticMeshRenderProgramName { ":/res/textured_static_mesh.vert", ":/res/textured_mesh.frag" };

const std::pair<std::string, std::string> shadowMeshRenderProgramName { ":/res/colored_mesh.vert", ":/res/empty.frag" };
const std::pair<std::string, std::string> shadowStaticMeshRenderProgramName { ":/res/colored_static_mesh.vert", ":/res/empty.frag" };


const std::string teapotModelName(":/res/teapot.fbx");
const std::string standardDiffuseTextureName(":/res/chess.png");
const std::string brdfLutTextureName(":/res/brdf_lut.json");

const std::pair<std::string, float> standardOpacityTexture = std::make_pair("StandardOpacity", 1.0f);
const std::pair<std::string, glm::vec3> standardNormalTexture = std::make_pair("StandardNormal", glm::vec3(0.5f, 0.5f, 1.0f));
const std::pair<std::string, float> standardMetallicTexture = std::make_pair("StandardMetallic", 0.0f);
const std::pair<std::string, float> standardRoughnessTexture = std::make_pair("StandardRoughness", 0.6f);



} // namespace
} // namespace

#endif // RESOURCES_H
