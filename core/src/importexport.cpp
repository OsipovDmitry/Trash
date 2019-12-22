#include <queue>
#include <stack>

#include "renderer.h"
#include "importexport.h"

namespace trash
{
namespace core
{

void push(std::ofstream& stream, float f)
{
    stream.write(reinterpret_cast<const char*>(&f), sizeof(float));
}
void push(std::ofstream& stream, uint16_t f)
{
    stream.write(reinterpret_cast<const char*>(&f), sizeof(uint16_t));
}
void push(std::ofstream& stream, int32_t f)
{
    stream.write(reinterpret_cast<const char*>(&f), sizeof(int32_t));
}
void push(std::ofstream& stream, uint32_t f)
{
    stream.write(reinterpret_cast<const char*>(&f), sizeof(uint32_t));
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
void push(std::ofstream& stream, const utils::Transform& f)
{
    push(stream, f.scale); push(stream, f.rotation); push(stream, f.translation);
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
void push(std::ofstream& stream, std::shared_ptr<Model::Material> f)
{
    push(stream, f->diffuseTexture.first);
    push(stream, f->normalTexture.first);
}
void push(std::ofstream& stream, std::shared_ptr<Model::Mesh> f)
{
    push(stream, f->mesh);
    push(stream, f->material);
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
void push(std::ofstream& stream, std::shared_ptr<Model> f)
{
    std::set<std::shared_ptr<Model::Mesh>> meshes;
    std::stack<std::shared_ptr<Model::Node>> nodes;
    if (f->rootNode) nodes.push(f->rootNode);
    while (!nodes.empty()) {
        auto node = nodes.top();
        nodes.pop();
        for (auto mesh : node->meshes) meshes.insert(mesh);
        for (auto child: node->children()) nodes.push(child);
    }
    push(stream, static_cast<uint16_t>(meshes.size()));
    for (auto mesh : meshes) push(stream, mesh);
    nodes.push(f->rootNode);
    while (!nodes.empty()) {
        auto node = nodes.top();
        nodes.pop();

        push(stream, node->transform);

        push(stream, static_cast<uint16_t>(node->meshes.size()));
        for (auto mesh : node->meshes)
            push(stream, static_cast<uint16_t>(std::distance(meshes.begin(), meshes.find(mesh))));

        push(stream, node->boneIndex);

        push(stream, static_cast<uint16_t>(node->children().size()));
        for (auto it = node->children().rbegin(); it != node->children().rend(); ++it)
            nodes.push(*it);
    }

    push(stream, static_cast<uint16_t>(f->animations.size()));
    for (auto& animation : f->animations)
    {
        push(stream, animation.first);
        push(stream, animation.second);
    }

    push(stream, static_cast<uint16_t>(f->boneTransforms.size()));
    for (auto& boneTransform : f->boneTransforms)
        push(stream, boneTransform);

    push(stream, static_cast<uint16_t>(f->boneNames.size()));
    for (auto& boneName : f->boneNames)
        push(stream, boneName);
}

void pull(std::ifstream& stream, float& f)
{
    stream.read(reinterpret_cast<char*>(&f), sizeof(float));
}
void pull(std::ifstream& stream, uint16_t& f)
{
    stream.read(reinterpret_cast<char*>(&f), sizeof(uint16_t));
}
void pull(std::ifstream& stream, int32_t& f)
{
    stream.read(reinterpret_cast<char*>(&f), sizeof(int32_t));
}
void pull(std::ifstream& stream, uint32_t& f)
{
    stream.read(reinterpret_cast<char*>(&f), sizeof(uint32_t));
}
void pull(std::ifstream& stream, std::string& f)
{
    uint16_t len;
    pull(stream, len);
    f.resize(len);
    stream.read(&f[0], len);
}
void pull(std::ifstream& stream, glm::vec3& f)
{
    pull(stream, f.x); pull(stream, f.y); pull(stream, f.z);
}
void pull(std::ifstream& stream, glm::quat& f)
{
    pull(stream, f.x); pull(stream, f.y); pull(stream, f.z); pull(stream, f.w);
}
void pull(std::ifstream& stream, utils::Transform& f)
{
    pull(stream, f.scale); pull(stream, f.rotation); pull(stream, f.translation);
}
void pull(std::ifstream& stream, std::shared_ptr<Buffer> f, int64_t size)
{
    stream.read(reinterpret_cast<char*>(f->map(0, size, GL_MAP_WRITE_BIT)), size);
    f->unmap();
}
void pull(std::ifstream& stream, std::shared_ptr<VertexBuffer>& f)
{
    uint32_t nv, nc;
    pull(stream, nv);
    pull(stream, nc);
    f = std::make_shared<VertexBuffer>(nv, nc, nullptr, GL_STATIC_DRAW);
    pull(stream, std::static_pointer_cast<Buffer>(f), nv * nc * sizeof(float));
}
void pull(std::ifstream& stream, std::shared_ptr<IndexBuffer>& f)
{
    uint32_t ni;
    GLenum pt;
    pull(stream, ni);
    pull(stream, pt);
    f = std::make_shared<IndexBuffer>(pt, ni, nullptr, GL_STATIC_DRAW);
    pull(stream, std::static_pointer_cast<Buffer>(f), ni * sizeof(uint32_t));
}
void pull(std::ifstream& stream, std::shared_ptr<Mesh>& f)
{
    f = std::make_shared<Mesh>();

    uint32_t numAttribs, numPrimitiveSets, attrib;
    pull(stream, numAttribs);
    for (uint32_t i = 0; i < numAttribs; ++i) {
        pull(stream, attrib);
        std::shared_ptr<VertexBuffer> vBuf;
        pull(stream, vBuf);
        f->declareVertexAttribute(castToVertexAttribute(attrib), vBuf);
    }
    pull(stream, numPrimitiveSets);
    for (uint32_t i = 0; i < numPrimitiveSets; ++i) {
        std::shared_ptr<IndexBuffer> iBuf;
        pull(stream, iBuf);
        f->attachIndexBuffer(iBuf);
    }
}
void pull(std::ifstream& stream, std::shared_ptr<Model::Material>& f)
{
    f = std::make_shared<Model::Material>();

    pull(stream, f->diffuseTexture.first);
    f->diffuseTexture.second = Renderer::instance().loadTexture(f->diffuseTexture.first);

    pull(stream, f->normalTexture.first);
    f->normalTexture.second = Renderer::instance().loadTexture(f->normalTexture.first);
}
void pull(std::ifstream& stream, std::shared_ptr<Model::Mesh>& f)
{
    f = std::make_shared<Model::Mesh>(nullptr, nullptr);
    pull(stream, f->mesh);
    pull(stream, f->material);
}
void pull(std::ifstream& stream, std::shared_ptr<Model::Animation>& a)
{
    a = std::make_shared<Model::Animation>(0.0f, 0.0f);
    pull(stream, a->framesPerSecond);
    pull(stream, a->duration);

    uint32_t numTransforms, nc;
    std::string boneName;
    pull(stream, numTransforms);
    for (uint32_t i = 0; i < numTransforms; ++i)
    {
        pull(stream, boneName);
        auto& transform = a->transforms[boneName];

        auto& scales = std::get<0>(transform);
        pull(stream, nc);
        scales.resize(nc);
        for (uint32_t s = 0; s < nc; ++s)
        {
            pull(stream, scales[s].first);
            pull(stream, scales[s].second);
        }

        auto& rotations = std::get<1>(transform);
        pull(stream, nc);
        rotations.resize(nc);
        for (uint32_t r = 0; r < nc; ++r)
        {
            pull(stream, rotations[r].first);
            pull(stream, rotations[r].second);
        }

        auto& translations = std::get<2>(transform);
        pull(stream, nc);
        translations.resize(nc);
        for (uint32_t t = 0; t < nc; ++t)
        {
            pull(stream, translations[t].first);
            pull(stream, translations[t].second);
        }
    }
}
void pull(std::ifstream& stream, std::shared_ptr<Model>& f)
{
    f = std::make_shared<Model>();

    std::vector<std::shared_ptr<Model::Mesh>> meshes;
    uint16_t numMeshes, meshIdx, numChildren, numAnimations, numBoneTransforms, numBoneNames;
    std::string animName;

    pull(stream, numMeshes);
    meshes.resize(numMeshes);
    for (auto& mesh : meshes)
        pull(stream, mesh);

    f->rootNode = std::make_shared<Model::Node>();
    std::stack<std::shared_ptr<Model::Node>> nodes;
    nodes.push(f->rootNode);
    while (!nodes.empty()) {
        auto node = nodes.top();
        nodes.pop();

        pull(stream, node->transform);
        pull(stream, numMeshes);
        node->meshes.resize(numMeshes);
        for (uint16_t i = 0; i < numMeshes; ++i)
        {
            pull(stream, meshIdx);
            node->meshes[i] = meshes[meshIdx];
        }
        pull(stream, node->boneIndex);

        pull(stream, numChildren);
        for (uint16_t i = 0; i < numChildren; ++i)
        {
            auto child = std::make_shared<Model::Node>();
            node->attach(child);
            nodes.push(child);
        }
    }

    pull(stream, numAnimations);
    for (uint16_t i = 0; i < numAnimations; ++i)
    {
        pull(stream, animName);
        std::shared_ptr<Model::Animation> anim;
        pull(stream, anim);
        f->animations.insert({animName, anim});
    }

    pull(stream, numBoneTransforms);
    f->boneTransforms.resize(numBoneTransforms);
    for (auto& boneTransform : f->boneTransforms)
        pull(stream, boneTransform);

    pull(stream, numBoneNames);
    f->boneNames.resize(numBoneNames);
    for (auto& boneName : f->boneNames)
        pull(stream, boneName);
}

} // namespace
} // namespace
