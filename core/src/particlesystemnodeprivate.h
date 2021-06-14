#ifndef PARTICLESYSTEMNODEPRIVATE_H
#define PARTICLESYSTEMNODEPRIVATE_H

#include <core/particlesystemnode.h>

#include "drawablenodeprivate.h"

namespace trash
{
namespace core
{

class ParticleSystemDrawable;
class Texture;

class ParticleSystemNodePrivate : public DrawableNodePrivate
{
protected:
    virtual uint32_t maxNumParticles() const = 0;
    virtual uint32_t numParticlesPerSecond() const = 0;

    virtual void createParticle(const glm::vec3& position, glm::vec3& velocity, uint32_t& lifetime) const = 0;
    virtual void moveParticle(float dtSec, glm::vec3& velocity, glm::vec3& position) const = 0;
    virtual void updateParticle(uint32_t lifeTime, glm::vec4& color, float& halfSize) const = 0;

    ParticleType m_particleType;
    BlendingType m_blendingType;
    std::shared_ptr<Texture> m_opacityMap;
    float m_opacityMapFps;


    ParticleSystemNodePrivate(Node&, std::shared_ptr<ParticleSystemNode::AbstractEmitter>);

public:
    struct ParticlesData
    {
        std::vector<glm::vec4> positionSize;
        std::vector<glm::vec4> color;
        std::vector<glm::vec3> velocity;
        std::vector<uint32_t> lifetime;
        std::vector<glm::vec2> frameNumber;
    };

    void doUpdate(uint64_t, uint64_t) override;

    std::shared_ptr<ParticleSystemNode::AbstractEmitter> emitter;
    std::shared_ptr<ParticleSystemDrawable> drawable;
    std::array<ParticlesData, 2u> particlesDataArray;
    uint8_t currentParticlesData;
    bool distanceAttenuationState;
    float distanceAttenuationValue;
    float timeNumberCounter;

private:
    static glm::vec2 calcFrameNumber(uint32_t, uint32_t, float);

};

} // namespace
} // namespace

#endif // PARTICLESYSTEMNODEPRIVATE_H
