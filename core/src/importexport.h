#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#include <fstream>
#include "renderer.h"

namespace trash
{
namespace core
{

void push(std::ofstream& stream, bool f);
void push(std::ofstream& stream, float f);
void push(std::ofstream& stream, uint16_t f);
void push(std::ofstream& stream, int32_t f);
void push(std::ofstream& stream, uint32_t f);
void push(std::ofstream& stream, const std::string& f);
void push(std::ofstream& stream, const glm::vec3& f);
void push(std::ofstream& stream, const glm::quat& f);
void push(std::ofstream& stream, const utils::Transform& f);
void push(std::ofstream& stream, std::shared_ptr<Buffer> f, int64_t size);
void push(std::ofstream& stream, std::shared_ptr<VertexBuffer> f);
void push(std::ofstream& stream, std::shared_ptr<IndexBuffer> f);
void push(std::ofstream& stream, std::shared_ptr<Mesh> f);
void push(std::ofstream& stream, std::shared_ptr<Model::Material> f);
void push(std::ofstream& stream, std::shared_ptr<Model::Mesh> f);
void push(std::ofstream& stream, std::shared_ptr<Model::Animation> a);
void push(std::ofstream& stream, std::shared_ptr<Model> f);

void pull(std::ifstream& stream, bool& f);
void pull(std::ifstream& stream, float& f);
void pull(std::ifstream& stream, uint16_t& f);
void pull(std::ifstream& stream, int32_t& f);
void pull(std::ifstream& stream, uint32_t& f);
void pull(std::ifstream& stream, std::string& f);
void pull(std::ifstream& stream, glm::vec3& f);
void pull(std::ifstream& stream, glm::quat& f);
void pull(std::ifstream& stream, utils::Transform& f);
void pull(std::ifstream& stream, std::shared_ptr<Buffer> f, int64_t size);
void pull(std::ifstream& stream, std::shared_ptr<VertexBuffer>& f);
void pull(std::ifstream& stream, std::shared_ptr<IndexBuffer>& f);
void pull(std::ifstream& stream, std::shared_ptr<Mesh>& f);
void pull(std::ifstream& stream, std::shared_ptr<Model::Material>& f);
void pull(std::ifstream& stream, std::shared_ptr<Model::Mesh>& f);
void pull(std::ifstream& stream, std::shared_ptr<Model::Animation>& a);
void pull(std::ifstream& stream, std::shared_ptr<Model>& f);

} // namespace
} // namespace

#endif // IMPORTEXPORT_H
