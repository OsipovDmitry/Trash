#ifndef PARTICLESYSTEMNODEPRIVATE_H
#define PARTICLESYSTEMNODEPRIVATE_H

#include <core/particlesystemnode.h>

#include "drawablenodeprivate.h"

namespace trash
{
namespace core
{

class Drawable;

class ParticleSystemNodePrivate : public DrawableNodePrivate
{
public:
    struct ParticlesData
    {
        std::vector<glm::vec4> positionSize;
        std::vector<glm::vec4> color;
        std::vector<glm::vec3> velocity;
        std::vector<uint32_t> lifetime;
    };

    ParticleSystemNodePrivate(Node&, std::shared_ptr<ParticleSystemNode::AbstractEmitter>, std::shared_ptr<ParticleSystemNode::AbstractUpdater>);

    void doUpdate(uint64_t, uint64_t) override;

    std::shared_ptr<ParticleSystemNode::AbstractEmitter> emitter;
    std::shared_ptr<ParticleSystemNode::AbstractUpdater> updater;
    std::shared_ptr<Drawable> drawable;
    ParticlesData particlesData;
};

} // namespace
} // namespace

#endif // PARTICLESYSTEMNODEPRIVATE_H
