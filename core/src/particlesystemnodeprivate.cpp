#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "particlesystemnodeprivate.h"
#include "renderer.h"
#include "drawables.h"

namespace trash
{
namespace core
{

ParticleSystemNodePrivate::ParticleSystemNodePrivate(Node& thisNode, std::shared_ptr<ParticleSystemNode::AbstractEmitter> e, std::shared_ptr<ParticleSystemNode::AbstractUpdater> u)
    : DrawableNodePrivate(thisNode)
    , emitter(e)
    , updater(u)
    , particlesData()
{
}

void ParticleSystemNodePrivate::doUpdate(uint64_t time, uint64_t dt)
{
    DrawableNodePrivate::doUpdate(time, dt);

    const uint32_t maxNumParticles = updater->maxNumParticles();

    if (!drawable)
    {
        particlesData.positionSize.resize(maxNumParticles);
        particlesData.color.resize(maxNumParticles);
        particlesData.velocity.resize(maxNumParticles);
        particlesData.lifetime.resize(maxNumParticles);

        for (size_t i = 0; i < maxNumParticles; ++i)
        {
            particlesData.positionSize[i] = glm::vec4((*emitter)(), 0.15f);
            particlesData.color[i] = glm::vec4(1.0f, 0.5f, 0.5f, 0.8f);
            particlesData.velocity[i] = glm::vec3(0.0f);
            particlesData.lifetime[i] = 0u;
        }

        static std::vector<glm::vec2> texCoords {
            glm::vec2(0.f, 0.f), glm::vec2(1.f, 0.f), glm::vec2(0.f, 1.f), glm::vec2(1.f, 1.f)
        };

        static std::vector<uint32_t> indices {0u, 1u, 2u, 3u};

        auto mesh = std::make_shared<Mesh>();
        mesh->declareVertexAttribute(VertexAttribute::Position,
                                     std::make_shared<VertexBuffer>(particlesData.positionSize.size(), 4u, glm::value_ptr(*particlesData.positionSize.data()), GL_DYNAMIC_DRAW),
                                     1u);
        mesh->declareVertexAttribute(VertexAttribute::Color,
                                     std::make_shared<VertexBuffer>(particlesData.color.size(), 4u, glm::value_ptr(*particlesData.color.data()), GL_DYNAMIC_DRAW),
                                     1u);
        mesh->declareVertexAttribute(VertexAttribute::TexCoord,
                                     std::make_shared<VertexBuffer>(texCoords.size(), 2u, glm::value_ptr(*texCoords.data()), GL_STATIC_DRAW));
        mesh->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_TRIANGLE_STRIP, indices.size(), indices.data(), GL_STATIC_DRAW));
        mesh->numInstances = maxNumParticles;

        drawable = std::make_shared<ParticleSystemDrawable>(mesh);
        addDrawable(drawable);
    }

    const glm::vec3& gravity = updater->gravity();
    const float dtSec = dt * 0.001f;

    for (size_t i = 0; i < maxNumParticles; ++i)
    {
        if (particlesData.lifetime[i] <= dt)
        {
            //
        }
        else
        {
            particlesData.lifetime[i] -= dt;
            particlesData.velocity[i] += gravity * dtSec;
            const glm::vec3 pos = glm::vec3(particlesData.positionSize[i]) + particlesData.velocity[i] * dtSec;
            particlesData.positionSize[i] = glm::vec4(pos, particlesData.positionSize[i].w);
        }
    }
}

} // namespace
} // namespace
