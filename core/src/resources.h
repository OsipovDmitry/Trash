#ifndef RESOURCES_H
#define RESOURCES_H

#include <string>

const std::pair<std::string, std::string> coloreredMeshRenderProgramName { ":/res/colored_mesh.vert", ":/res/colored_mesh.frag" };
const std::pair<std::string, std::string> coloreredStaticMeshRenderProgramName { ":/res/colored_static_mesh.vert", ":/res/colored_mesh.frag" };

const std::pair<std::string, std::string> texturedMeshRenderProgramName { ":/res/textured_mesh.vert", ":/res/textured_mesh.frag" };
const std::pair<std::string, std::string> texturedStaticMeshRenderProgramName { ":/res/textured_static_mesh.vert", ":/res/textured_mesh.frag" };

const std::string standardDiffuseTextureName(":/res/chess.png");
const std::string standardNormalTextureName(":/res/normal.png");

#endif // RESOURCES_H
