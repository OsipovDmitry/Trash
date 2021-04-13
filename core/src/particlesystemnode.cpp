#include <random>

#include <utils/random.h>

#include <core/particlesystemnode.h>

#include "particlesystemnodeprivate.h"

namespace trash
{
namespace core
{

class PointEmitter : public ParticleSystemNode::AbstractEmitter
{
public:
    PointEmitter() : ParticleSystemNode::AbstractEmitter() {}
    glm::vec3 operator()() override { return glm::vec3(0.f); }
};

class RectEmitter : public ParticleSystemNode::AbstractEmitter
{
public:
    RectEmitter(const glm::vec2& halfSize)
        : ParticleSystemNode::AbstractEmitter()
        , m_halfSize(halfSize)
    {}
    glm::vec3 operator()() override { return glm::vec3(utils::random(-m_halfSize.x, m_halfSize.x), 0.0f, utils::random(-m_halfSize.y, m_halfSize.y)); }

private:
    glm::vec2 m_halfSize;
};

class CircleEmitter : public ParticleSystemNode::AbstractEmitter
{
public:
    CircleEmitter(float radius)
        : ParticleSystemNode::AbstractEmitter()
        , m_radius(radius)
    {}
    glm::vec3 operator()() override {
        const float a = utils::random(0.0f, glm::two_pi<float>());
        const float r = m_radius * glm::sqrt(utils::random());
        return glm::vec3(r * glm::cos(a), 0.0f, r * glm::sin(a));
    }

private:
    float m_radius;
};

class BoxEmitter : public ParticleSystemNode::AbstractEmitter
{
public:
    BoxEmitter(const glm::vec3& halfSize)
        : ParticleSystemNode::AbstractEmitter()
        , m_halfSize(halfSize)
    {}
    glm::vec3 operator()() override { return glm::vec3(utils::random(-m_halfSize.x, m_halfSize.x), utils::random(-m_halfSize.y, m_halfSize.y), utils::random(-m_halfSize.z, m_halfSize.z)); }

private:
    glm::vec3 m_halfSize;
};

class SphereEmitter : public ParticleSystemNode::AbstractEmitter
{
public:
    SphereEmitter(float radius)
        : ParticleSystemNode::AbstractEmitter()
        , m_radius(radius)
    {}
    glm::vec3 operator()() override {
        const float a = utils::random(0.0f, glm::two_pi<float>());
        const float b = utils::random(-glm::half_pi<float>(), glm::half_pi<float>());
        const float r = m_radius * glm::sqrt(utils::random());
        return glm::vec3(r * glm::cos(a) * glm::cos(b), r * glm::sin(b), r * glm::sin(a) * glm::cos(b));
    }

private:
    float m_radius;
};

class FireUpdater : public ParticleSystemNode::AbstractUpdater
{
public:
    FireUpdater()
        : ParticleSystemNode::AbstractUpdater()
        , m_gravity(0.f, .7f, 0.f)
    {}

    uint32_t maxNumParticles() const override { return 500u; }
    uint32_t numParticlesPerSecond() const override { return 30u; }
    const glm::vec3& gravity() const override { return m_gravity; }

private:
    glm::vec3 m_gravity;
};

std::shared_ptr<ParticleSystemNode::AbstractEmitter> ParticleSystemNode::AbstractEmitter::buildPointEmitter()
{
    return std::make_shared<PointEmitter>();
}

std::shared_ptr<ParticleSystemNode::AbstractEmitter> ParticleSystemNode::AbstractEmitter::buildRectEmitter(const glm::vec2& halfSize)
{
    return std::make_shared<RectEmitter>(halfSize);
}

std::shared_ptr<ParticleSystemNode::AbstractEmitter> ParticleSystemNode::AbstractEmitter::buildCircleEmitter(float radius)
{
    return std::make_shared<CircleEmitter>(radius);
}

std::shared_ptr<ParticleSystemNode::AbstractEmitter> ParticleSystemNode::AbstractEmitter::buildBoxEmitter(const glm::vec3& halfSize)
{
    return std::make_shared<BoxEmitter>(halfSize);
}

std::shared_ptr<ParticleSystemNode::AbstractEmitter> ParticleSystemNode::AbstractEmitter::buildSphereEmitter(float radius)
{
    return std::make_shared<SphereEmitter>(radius);
}

ParticleSystemNode::AbstractEmitter::AbstractEmitter()
{
}

std::shared_ptr<ParticleSystemNode::AbstractUpdater> ParticleSystemNode::AbstractUpdater::buildFireUpdater()
{
    return std::make_shared<FireUpdater>();
}

ParticleSystemNode::AbstractUpdater::AbstractUpdater()
{
}

ParticleSystemNode::ParticleSystemNode(std::shared_ptr<AbstractEmitter> emitter, std::shared_ptr<AbstractUpdater> updater)
    : DrawableNode(new ParticleSystemNodePrivate(*this, emitter, updater))
{
    setIntersectionMode(IntersectionMode::None);

    auto& nodePrivate = m();
    nodePrivate.lightIndices->isEnabled = false;
    nodePrivate.areShadowsEnabled = false;
}

} // namespace
} // namespace
