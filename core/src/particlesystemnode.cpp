#include <random>

#include <glm/gtc/color_space.hpp>

#include <utils/random.h>
#include <utils/interpolator.h>

#include <core/particlesystemnode.h>

#include "particlesystemnodeprivate.h"
#include "drawables.h"
#include "renderer.h"
#include "resources.h"

namespace trash
{
namespace core
{

class PointEmitter : public ParticleSystemNode::AbstractEmitter
{
public:
    glm::vec3 operator()() override { return glm::vec3(0.f); }
};

class QuadEmitter : public ParticleSystemNode::AbstractEmitter
{
public:
    glm::vec3 operator()() override { return glm::vec3(utils::random(-.5f, .5f), 0.f, utils::random(-.5f, .5f)); }
};

class CircleEmitter : public ParticleSystemNode::AbstractEmitter
{
public:
    glm::vec3 operator()() override {
        const float a = utils::random(0.0f, glm::two_pi<float>());
        const float r = .5f * glm::sqrt(utils::random());
        return glm::vec3(r * glm::cos(a), 0.0f, r * glm::sin(a));
    }
};

class BoxEmitter : public ParticleSystemNode::AbstractEmitter
{
public:
    glm::vec3 operator()() override { return glm::vec3(utils::random(-.5f, .5f), utils::random(-.5f, .5f), utils::random(-.5f, .5f)); }
};

class SphereEmitter : public ParticleSystemNode::AbstractEmitter
{
public:
    glm::vec3 operator()() override {
        const float a = utils::random(0.0f, glm::two_pi<float>());
        const float b = utils::random(-glm::half_pi<float>(), glm::half_pi<float>());
        const float r = .5f * glm::sqrt(utils::random());
        return glm::vec3(r * glm::cos(a) * glm::cos(b), r * glm::sin(b), r * glm::sin(a) * glm::cos(b));
    }
};

class FireParticleSystemNodePrivate : public ParticleSystemNodePrivate
{
public:
    FireParticleSystemNodePrivate(Node& n, std::shared_ptr<ParticleSystemNode::AbstractEmitter> e)
        : ParticleSystemNodePrivate(n, e)
        , m_opacityInterpolator()
        , m_color(glm::convertSRGBToLinear(glm::vec3(1.0f, 0.2f, 0.05f)))
    {
        m_particleType = ParticleType::SoftCircle;
        m_blendingType = BlendingType::Additive;

        m_opacityInterpolator.addValue(0.f, 0.f);
        m_opacityInterpolator.addValue(1.2f, .8f);
    }

    uint32_t maxNumParticles() const override { return 1700u; }
    uint32_t numParticlesPerSecond() const override { return 800u; }

    void createParticle(const glm::vec3& /*position*/, glm::vec3& velocity, uint32_t& lifetime) const override {
        velocity = glm::vec3(0.0f, 1.0f, 0.0f);
        lifetime = utils::random(0.5f, 1.0f) * 2500u; // [1250..2500]
    }

    void moveParticle(float dtSec, glm::vec3& velocity, glm::vec3& position) const override {
        const glm::vec3 gravity = glm::normalize(-glm::vec3(position.x, -1.f, position.z)) * 0.1f;

        velocity += dtSec * gravity;
        position += dtSec * velocity;
    }

    void updateParticle(uint32_t lifeTime, glm::vec4& color, float& halfSize) const override {
        color = glm::vec4(m_color, m_opacityInterpolator.interpolate(lifeTime * .001f));
        halfSize = 0.2f;
    }

protected:
    utils::LinearInterpolator<float> m_opacityInterpolator;
    const glm::vec3 m_color;
};

class FireParticleSystemNode : public ParticleSystemNode
{
public:
    FireParticleSystemNode(std::shared_ptr<ParticleSystemNode::AbstractEmitter> e)
        : ParticleSystemNode(new FireParticleSystemNodePrivate(*this, e))
    {}
};

class SmokeParticleSystemNodePrivate : public ParticleSystemNodePrivate
{
public:
    SmokeParticleSystemNodePrivate(Node& n, std::shared_ptr<ParticleSystemNode::AbstractEmitter> e)
        : ParticleSystemNodePrivate(n, e)
        , m_sizeInterpolator()
        , m_opacityInterpolator()
        , m_colorInterpolator()
    {
        m_particleType = ParticleType::Quad;
        m_blendingType = BlendingType::Alpha;
        m_opacityMap = Renderer::instance().loadTexture(smokeOpacityMapName);
        m_opacityMapFps = 6.0f;

        m_sizeInterpolator.addValue(0.0f, 0.65f);
        m_sizeInterpolator.addValue(3.0f, 0.5f);

        m_opacityInterpolator.addValue(0.0f, 0.0f);
        m_opacityInterpolator.addValue(2.5f, 0.3f);
        m_opacityInterpolator.addValue(3.5f, 1.0f);

        m_colorInterpolator.addValue(0.0f, glm::convertSRGBToLinear(glm::vec3(0.7f)));
        m_colorInterpolator.addValue(1.5f, glm::convertSRGBToLinear(glm::vec3(0.35f)));
        m_colorInterpolator.addValue(3.5f, glm::convertSRGBToLinear(glm::vec3(0.15f)));
    }

    uint32_t maxNumParticles() const override { return 100u; }
    uint32_t numParticlesPerSecond() const override { return 20u; }

    void createParticle(const glm::vec3& /*position*/, glm::vec3& velocity, uint32_t& lifetime) const override {
        velocity = glm::vec3(0.0f, 0.7f, 0.0f);
        lifetime = utils::random(0.7f, 1.0f) * 5000u; //[3500..5000]
    }

    void moveParticle(float dtSec, glm::vec3& velocity, glm::vec3& position) const override {
        const glm::vec3 gravity = glm::normalize(glm::vec3(position.x, .8f, position.z)) * 0.1f;

        velocity += dtSec * gravity;
        position += dtSec * velocity;
    }

    void updateParticle(uint32_t lifeTime, glm::vec4& color, float& halfSize) const override {
        const float lifeTimeSec = lifeTime * .001f;
        color = glm::vec4(m_colorInterpolator.interpolate(lifeTimeSec), m_opacityInterpolator.interpolate(lifeTimeSec));
        halfSize = m_sizeInterpolator.interpolate(lifeTimeSec);
    }

protected:
    utils::LinearInterpolator<float> m_sizeInterpolator;
    utils::LinearInterpolator<float> m_opacityInterpolator;
    utils::LinearInterpolator<glm::vec3> m_colorInterpolator;
};

class SmokeParticleSystemNode : public ParticleSystemNode
{
public:
    SmokeParticleSystemNode(std::shared_ptr<ParticleSystemNode::AbstractEmitter> e)
        : ParticleSystemNode(new SmokeParticleSystemNodePrivate(*this, e))
    {}
};

std::shared_ptr<ParticleSystemNode::AbstractEmitter> ParticleSystemNode::AbstractEmitter::buildPointEmitter()
{
    return std::make_shared<PointEmitter>();
}

std::shared_ptr<ParticleSystemNode::AbstractEmitter> ParticleSystemNode::AbstractEmitter::buildQuadEmitter()
{
    return std::make_shared<QuadEmitter>();
}

std::shared_ptr<ParticleSystemNode::AbstractEmitter> ParticleSystemNode::AbstractEmitter::buildCircleEmitter()
{
    return std::make_shared<CircleEmitter>();
}

std::shared_ptr<ParticleSystemNode::AbstractEmitter> ParticleSystemNode::AbstractEmitter::buildBoxEmitter()
{
    return std::make_shared<BoxEmitter>();
}

std::shared_ptr<ParticleSystemNode::AbstractEmitter> ParticleSystemNode::AbstractEmitter::buildSphereEmitter()
{
    return std::make_shared<SphereEmitter>();
}

ParticleSystemNode::AbstractEmitter::AbstractEmitter()
{
}

ParticleSystemNode::ParticleSystemNode(ParticleSystemNodePrivate *nodePrivate)
    : DrawableNode(nodePrivate)
{
    setIntersectionMode(IntersectionMode::None);

    nodePrivate->lightIndices.isEnabled = false;
    nodePrivate->areShadowsEnabled = false;
}

void ParticleSystemNode::setDistanceAttenuation(bool state, float value)
{
    auto& nodePrivate = m();
    nodePrivate.distanceAttenuationState = state;
    nodePrivate.distanceAttenuationValue = value;
    if (nodePrivate.drawable)
        nodePrivate.drawable->dirtyCache();
}

std::shared_ptr<ParticleSystemNode> ParticleSystemNode::buildFire(std::shared_ptr<AbstractEmitter> e)
{
    return std::make_shared<FireParticleSystemNode>(e);
}

std::shared_ptr<ParticleSystemNode> ParticleSystemNode::buildSmoke(std::shared_ptr<AbstractEmitter> e)
{
    return std::make_shared<SmokeParticleSystemNode>(e);
}

} // namespace
} // namespace
