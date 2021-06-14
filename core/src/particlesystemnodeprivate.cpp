#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <utils/random.h>

#include <core/scene.h>

#include "particlesystemnodeprivate.h"
#include "renderer.h"
#include "drawables.h"

namespace trash
{
namespace core
{

ParticleSystemNodePrivate::ParticleSystemNodePrivate(Node& thisNode, std::shared_ptr<ParticleSystemNode::AbstractEmitter> e)
    : DrawableNodePrivate(thisNode)
    , m_particleType(ParticleType::SoftCircle)
    , m_blendingType(BlendingType::Additive)
    , m_opacityMap()
    , m_opacityMapFps(16.f)
    , emitter(e)
    , particlesDataArray()
    , currentParticlesData(0)
    , distanceAttenuationState(false)
    , distanceAttenuationValue(1.0f)
    , timeNumberCounter(0.f)
{
}

void ParticleSystemNodePrivate::doUpdate(uint64_t time, uint64_t dt)
{
    DrawableNodePrivate::doUpdate(time, dt);

    const uint32_t numParticles = maxNumParticles();

    auto& particlesData = particlesDataArray[currentParticlesData];

    if (!drawable)
    {
        particlesData.positionSize.resize(numParticles);
        particlesData.color.resize(numParticles);
        particlesData.velocity.resize(numParticles);
        particlesData.lifetime.resize(numParticles);
        particlesData.frameNumber.resize(numParticles);

        for (size_t i = 0; i < numParticles; ++i)
            particlesData.lifetime[i] = 0u;

        static std::vector<glm::vec2> texCoords {
            glm::vec2(0.f, 0.f), glm::vec2(1.f, 0.f), glm::vec2(0.f, 1.f), glm::vec2(1.f, 1.f)
        };

        static std::vector<uint32_t> indices {0u, 1u, 2u, 3u};

        auto mesh = std::make_shared<Mesh>();
        mesh->declareVertexAttribute(VertexAttribute::Position,
                                     std::make_shared<VertexBuffer>(numParticles, decltype(particlesData.positionSize)::value_type::length(), nullptr, GL_DYNAMIC_DRAW),
                                     1u);
        mesh->declareVertexAttribute(VertexAttribute::Color,
                                     std::make_shared<VertexBuffer>(numParticles, decltype(particlesData.color)::value_type::length(), nullptr, GL_DYNAMIC_DRAW),
                                     1u);
        mesh->declareVertexAttribute(VertexAttribute::BonesIDs, // frameNumber
                                     std::make_shared<VertexBuffer>(numParticles, decltype(particlesData.frameNumber)::value_type::length(), nullptr, GL_DYNAMIC_DRAW),
                                     1u);
        mesh->declareVertexAttribute(VertexAttribute::TexCoord,
                                     std::make_shared<VertexBuffer>(texCoords.size(), 2u, glm::value_ptr(*texCoords.data()), GL_STATIC_DRAW));
        mesh->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_TRIANGLE_STRIP, indices.size(), indices.data(), GL_STATIC_DRAW));
        mesh->numInstances = numParticles;

        drawable = std::make_shared<ParticleSystemDrawable>(mesh,
                                                            std::cref(m_particleType),
                                                            std::cref(m_blendingType),
                                                            std::cref(distanceAttenuationState),
                                                            std::cref(distanceAttenuationValue),
                                                            m_opacityMap);
        addDrawable(drawable);
    }

    const float dtSec = dt * 0.001f;
    const float frameSec = 1.0f / m_opacityMapFps;

    uint32_t numFrames = 1;
    if (m_opacityMap)
        numFrames = m_opacityMap->size[2];

    for (size_t i = 0; i < numParticles; ++i)
    {
        if (particlesData.lifetime[i] <= dt)
        {
            particlesData.lifetime[i] = 0u;
            particlesData.positionSize[i].w = -1.f;
        }
        else
        {
            particlesData.lifetime[i] -= dt;
            particlesData.frameNumber[i] = calcFrameNumber(particlesData.lifetime[i], numFrames, m_opacityMapFps);

            moveParticle(dtSec, particlesData.velocity[i], reinterpret_cast<glm::vec3&>(particlesData.positionSize[i]));
            updateParticle(particlesData.lifetime[i], particlesData.color[i], particlesData.positionSize[i].w);


        }
    }

    timeNumberCounter += dtSec;
    const float newParticleTime = 1.f / numParticlesPerSecond();
    for (size_t i = 0; (i < numParticles) && (timeNumberCounter >= newParticleTime); ++i)
        if (particlesData.lifetime[i] == 0u)
        {
            particlesData.positionSize[i] = glm::vec4((*emitter)(), 1.0f);
            createParticle(particlesData.positionSize[i], particlesData.velocity[i], particlesData.lifetime[i]);

            const uint32_t ddt = static_cast<uint32_t>(utils::random() * (dt % particlesData.lifetime[i]) + .5f);
            particlesData.lifetime[i] -= ddt;
            particlesData.frameNumber[i] = calcFrameNumber(particlesData.lifetime[i], numFrames, m_opacityMapFps);

            moveParticle(ddt * .001f, particlesData.velocity[i], reinterpret_cast<glm::vec3&>(particlesData.positionSize[i]));
            updateParticle(particlesData.lifetime[i], particlesData.color[i], particlesData.positionSize[i].w);

            timeNumberCounter -= newParticleTime;
        }


    auto& bb = drawable->mesh()->boundingBox;
    bb = utils::BoundingBox();

    for (size_t i = 0; i < numParticles; ++i)
        if (particlesData.lifetime[i] > 0u)
            bb += utils::BoundingBox::fromCenterHalfSize(reinterpret_cast<glm::vec3&>(particlesData.positionSize[i]), glm::vec3(particlesData.positionSize[i].w));

    dirtyLocalBoundingBox();

    if (m_blendingType == BlendingType::Alpha)
    {
        auto* scene = getScene();
        if (scene)
        {
            const glm::vec3 viewPosition =
                    getGlobalTransform().inverted() *
                    glm::vec3(glm::inverse(scene->viewMatrix()) * glm::vec4(0.f, 0.f, 0.f, 1.f));

            std::vector<std::pair<float, size_t>> dists;
            dists.reserve(numParticles);
            for (size_t i = 0; i < numParticles; ++i)
                dists.push_back(std::make_pair(glm::length2(reinterpret_cast<const glm::vec3&>(particlesData.positionSize[i]) - viewPosition), i));

            static const auto comparator = [](const std::pair<float, size_t>& v1, const std::pair<float, size_t>& v2) {
                return v1.first > v2.first;
            };
            std::sort(dists.begin(), dists.end(), comparator);

            currentParticlesData = 1u - currentParticlesData;
            auto& copyData = particlesDataArray[currentParticlesData];

            copyData.positionSize.resize(numParticles);
            copyData.color.resize(numParticles);
            copyData.velocity.resize(numParticles);
            copyData.lifetime.resize(numParticles);
            copyData.frameNumber.resize(numParticles);

            for (size_t i = 0; i < numParticles; ++i)
            {
                copyData.positionSize[i] = particlesData.positionSize[dists[i].second];
                copyData.color[i] = particlesData.color[dists[i].second];
                copyData.velocity[i] = particlesData.velocity[dists[i].second];
                copyData.lifetime[i] = particlesData.lifetime[dists[i].second];
                copyData.frameNumber[i] = particlesData.frameNumber[dists[i].second];
            }
        }
    }

    drawable->mesh()->vertexBuffer(VertexAttribute::Position)->setSubData(0u, numParticles * sizeof(decltype(particlesData.positionSize)::value_type), glm::value_ptr(particlesDataArray[currentParticlesData].positionSize[0]));
    drawable->mesh()->vertexBuffer(VertexAttribute::Color)->setSubData(0u, numParticles * sizeof(decltype(particlesData.color)::value_type), glm::value_ptr(particlesDataArray[currentParticlesData].color[0]));

    if (m_opacityMap)
        drawable->mesh()->vertexBuffer(VertexAttribute::BonesIDs)->setSubData(0u, numParticles * sizeof(decltype(particlesData.frameNumber)::value_type), particlesDataArray[currentParticlesData].frameNumber.data());
}

glm::vec2 ParticleSystemNodePrivate::calcFrameNumber(uint32_t lifetime, uint32_t numFrames, float fps)
{
    const float frameNumberFloat = lifetime * .001f * fps;
    const uint32_t frameNumber = static_cast<uint32_t>(frameNumberFloat);
    const float coef = frameNumberFloat - frameNumber;
    return glm::vec2(numFrames - (frameNumber % numFrames) - 1u, 1.f - coef);

}

} // namespace
} // namespace
