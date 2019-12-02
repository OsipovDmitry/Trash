#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#include <fstream>
#include "renderer.h"

#include <queue>
#include <stack>
#include "importexport.h"

void push(std::ofstream& stream, float f);
void push(std::ofstream& stream, uint16_t f);
void push(std::ofstream& stream, int32_t f);
void push(std::ofstream& stream, uint32_t f);
void push(std::ofstream& stream, const std::string& f);
void push(std::ofstream& stream, const glm::vec3& f);
void push(std::ofstream& stream, const glm::quat& f);
void push(std::ofstream& stream, const Transform& f);
void push(std::ofstream& stream, std::shared_ptr<Buffer> f, int64_t size);
void push(std::ofstream& stream, std::shared_ptr<VertexBuffer> f);
void push(std::ofstream& stream, std::shared_ptr<IndexBuffer> f);
void push(std::ofstream& stream, std::shared_ptr<Mesh> f);
void push(std::ofstream& stream, std::shared_ptr<Model::Material> f);
void push(std::ofstream& stream, std::shared_ptr<Model::Mesh> f);
void push(std::ofstream& stream, std::shared_ptr<Model::Animation> a);
void push(std::ofstream& stream, std::shared_ptr<Model> f);


#endif // IMPORTEXPORT_H
