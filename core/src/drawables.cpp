#include <utils/frustum.h>

#include "drawables.h"

SkeletalMeshDrawable::SkeletalMeshDrawable(std::shared_ptr<RenderProgram> rp, std::shared_ptr<Model::Mesh> m, std::shared_ptr<UniformBuffer> mb)
    : program(rp)
    , skeletalMesh(m)
    , bufferData(mb)
{
}

std::shared_ptr<RenderProgram> SkeletalMeshDrawable::renderProgram() const
{
    return program;
}

std::shared_ptr<Mesh> SkeletalMeshDrawable::mesh() const
{
    return skeletalMesh->mesh;
}

void SkeletalMeshDrawable::setup()
{
    auto& renderer = Renderer::instance();

    if (skeletalMesh->material && skeletalMesh->material->diffuseTexture)
    {
        program->setUniform(program->uniformLocation("u_diffuseMap"), 0);
        renderer.bindTexture(skeletalMesh->material->diffuseTexture, 0);
    }

    if (bufferData)
    {
        renderer.bindUniformBuffer(bufferData, program->uniformBufferIndexByName("u_ModelData"));
    }
}

SphereDrawable::SphereDrawable(std::shared_ptr<RenderProgram> rp, uint32_t segs, const BoundingSphere &bs, const glm::vec4& color)
    : renderProgram_(rp)
    , color_(color)
{
    std::vector<float> vertices(3 * (segs+1) * (segs * segs));
    std::vector<uint32_t> indices(4 * (segs * segs * segs));

    for (uint32_t a = 0; a <= segs; ++a)
    {
        float angleA = glm::pi<float>() * (static_cast<float>(a) / segs - .5f);
        float sinA = glm::sin(angleA);
        float cosA = glm::cos(angleA);

        for (uint32_t b = 0; b < segs*segs; ++b)
        {
            float angleB = 2.f * glm::pi<float>() * static_cast<float>(b) / (segs*segs - 1);
            float sinB = glm::sin(angleB);
            float cosB = glm::cos(angleB);

            vertices[3 * (a * segs * segs + b) + 0] = bs.w * cosA * sinB + bs.x;
            vertices[3 * (a * segs * segs + b) + 1] = bs.w * sinA + bs.y;
            vertices[3 * (a * segs * segs + b) + 2] = bs.w * cosA * cosB + bs.z;

            if ((a < segs) && (b < (segs*segs - 1)))
            {
                indices[4 * (a*(segs*segs) + b) + 0] = a*(segs*segs) + b;
                indices[4 * (a*(segs*segs) + b) + 1] = a*(segs*segs) + b + 1;
                indices[4 * (a*(segs*segs) + b) + 2] = a*(segs*segs) + b;
                indices[4 * (a*(segs*segs) + b) + 3] = (a+1)*(segs*segs) + b;
            }
        }
    }

    auto vbo = std::make_shared<VertexBuffer>(vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    auto ibo = std::make_shared<IndexBuffer>(GL_LINES, indices.size(), indices.data(), GL_STATIC_DRAW);

    mesh_ = std::make_shared<Mesh>();
    mesh_->attachVertexBuffer(VertexAttribute::Position, vbo, 3, 3 * sizeof(float), 0);
    mesh_->attachIndexBuffer(ibo);
}

std::shared_ptr<RenderProgram> SphereDrawable::renderProgram() const
{
    return renderProgram_;
}

std::shared_ptr<Mesh> SphereDrawable::mesh() const
{
    return mesh_;
}

void SphereDrawable::setup()
{
    renderProgram_->setUniform(renderProgram_->uniformLocation("u_color"), color_);
}

FrustumDrawable::FrustumDrawable(std::shared_ptr<RenderProgram> rp, const Frustum &frustum, const glm::vec4& color)
    : renderProgram_(rp)
    , color_(color)
{
    static auto intersectPlanes = [](const Plane& p0, const Plane& p1, const Plane& p2) -> glm::vec3 {
        glm::vec3 bxc = glm::cross(p1.normal(), p2.normal());
        glm::vec3 cxa = glm::cross(p2.normal(), p0.normal());
        glm::vec3 axb = glm::cross(p0.normal(), p1.normal());
        glm::vec3 r = -p0.dist() * bxc - p1.dist() * cxa - p2.dist() * axb;
        return -r * (1.0f / glm::dot(p0.normal(), bxc));
    };

    std::vector<glm::vec3> vertices({
        intersectPlanes(frustum.planes[4], frustum.planes[0], frustum.planes[2]),
        intersectPlanes(frustum.planes[4], frustum.planes[1], frustum.planes[2]),
        intersectPlanes(frustum.planes[4], frustum.planes[1], frustum.planes[3]),
        intersectPlanes(frustum.planes[4], frustum.planes[0], frustum.planes[3]),
        intersectPlanes(frustum.planes[5], frustum.planes[0], frustum.planes[2]),
        intersectPlanes(frustum.planes[5], frustum.planes[1], frustum.planes[2]),
        intersectPlanes(frustum.planes[5], frustum.planes[1], frustum.planes[3]),
        intersectPlanes(frustum.planes[5], frustum.planes[0], frustum.planes[3]) });

    std::vector<uint32_t> indices({ 0,1, 1,2, 2,3, 3,0, 4,5, 5,6, 6,7, 7,4, 0,4, 1,5, 2,6, 3,7 });

    auto vbo = std::make_shared<VertexBuffer>(vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    auto ibo = std::make_shared<IndexBuffer>(GL_LINES, indices.size(), indices.data(), GL_STATIC_DRAW);

    mesh_ = std::make_shared<Mesh>();
    mesh_->attachVertexBuffer(VertexAttribute::Position, vbo, 3, sizeof(glm::vec3), 0);
    mesh_->attachIndexBuffer(ibo);
}

std::shared_ptr<RenderProgram> FrustumDrawable::renderProgram() const
{
    return renderProgram_;
}

std::shared_ptr<Mesh> FrustumDrawable::mesh() const
{
    return mesh_;
}

void FrustumDrawable::setup()
{
    renderProgram_->setUniform(renderProgram_->uniformLocation("u_color"), color_);
}
