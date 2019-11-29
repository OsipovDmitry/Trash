#include "importexport.h"

void push(std::ofstream& stream, float f)
{
    stream.write(reinterpret_cast<const char*>(&f), sizeof(f));
}
void push(std::ofstream& stream, uint16_t f)
{
    stream.write(reinterpret_cast<const char*>(&f), sizeof(f));
}
void push(std::ofstream& stream, uint32_t f)
{
    stream.write(reinterpret_cast<const char*>(&f), sizeof(f));
}
void push(std::ofstream& stream, const std::string& f)
{
    push(stream, static_cast<uint16_t>(f.size()));
    stream.write(f.c_str(), f.size());
}
void push(std::ofstream& stream, const glm::vec3& f)
{
    push(stream, f.x); push(stream, f.y); push(stream, f.z);
}
void push(std::ofstream& stream, const glm::quat& f)
{
    push(stream, f.x); push(stream, f.y); push(stream, f.z); push(stream, f.w);
}
void push(std::ofstream& stream, std::shared_ptr<Buffer> f, int64_t size)
{
    stream.write(reinterpret_cast<const char*>(f->map(0, size, GL_MAP_READ_BIT)), size);
    f->unmap();
}
void push(std::ofstream& stream, std::shared_ptr<VertexBuffer> f)
{
    push(stream, f->numVertices);
    push(stream, f->numComponents);
    push(stream, std::static_pointer_cast<Buffer>(f), f->numVertices * f->numComponents * sizeof(float));
}
void push(std::ofstream& stream, std::shared_ptr<IndexBuffer> f)
{
    push(stream, f->numIndices);
    push(stream, f->primitiveType);
    push(stream, std::static_pointer_cast<Buffer>(f), f->numIndices * sizeof(uint32_t));
}
void push(std::ofstream& stream, std::shared_ptr<Mesh> f)
{
    push(stream, static_cast<uint32_t>(f->attributesDeclaration.size()));
    for (auto& attrib : f->attributesDeclaration) {
        push(stream, castFromVertexAttribute(attrib.first));
        push(stream, attrib.second);
    }
    push(stream, static_cast<uint32_t>(f->indexBuffers.size()));
    for (auto& ib : f->indexBuffers)
        push(stream, ib);
}
void push(std::ofstream& stream, std::shared_ptr<Model::Animation> a)
{
    push(stream, a->framesPerSecond);
    push(stream, a->duration);
    push(stream, static_cast<uint32_t>(a->transforms.size()));

    for (auto& transform : a->transforms)
    {
        push(stream, transform.first);

        auto& scales = std::get<0>(transform.second);
        push(stream, static_cast<uint32_t>(scales.size()));
        for (auto& scale : scales)
        {
            push(stream, scale.first);
            push(stream, scale.second);
        }

        auto& rotations = std::get<1>(transform.second);
        push(stream, static_cast<uint32_t>(rotations.size()));
        for (auto& rotation : rotations)
        {
            push(stream, rotation.first);
            push(stream, rotation.second);
        }

        auto& translations = std::get<2>(transform.second);
        push(stream, static_cast<uint32_t>(translations.size()));
        for (auto& translation : translations)
        {
            push(stream, translation.first);
            push(stream, translation.second);
        }
    }
}
