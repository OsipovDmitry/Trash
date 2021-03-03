#ifndef UTILS_H
#define UTILS_H

#include <memory>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <core/forwarddecl.h>
#include <utils/forwarddecl.h>

class QImage;

namespace trash
{
namespace core
{

struct Mesh;
struct Font;

glm::vec4 toLinearRGB(const glm::vec4&);
glm::vec4 toSRGB(const glm::vec4&);

bool diffuseSpecularGlossinessToBaseColorMetallicRoughness(const QImage&, const QImage&, const QImage&, const QImage&, QImage&, QImage&, QImage&);

std::shared_ptr<Mesh> buildLineMesh(const std::vector<glm::vec3>&, const std::vector<glm::vec3>&, bool);
std::shared_ptr<Mesh> buildSphereMesh(uint32_t, const utils::BoundingSphere&, bool);
std::shared_ptr<Mesh> buildBoxMesh(const utils::BoundingBox&, bool);
std::shared_ptr<Mesh> buildFrustumMesh(const utils::Frustum&);
std::shared_ptr<Mesh> buildConeMesh(uint32_t, float, float, bool);
std::shared_ptr<Mesh> buildPlaneMesh();
std::shared_ptr<Mesh> buildTextMesh(std::shared_ptr<Font>, const std::string&, TextNodeAlignment, TextNodeAlignment, float);

int32_t numberOfMipmaps(int32_t, int32_t);

} // namespace
} // namespace


#endif // UTILS_H
