#ifndef PARTICLESYSTEMNODE_H
#define PARTICLESYSTEMNODE_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <core/drawablenode.h>

namespace trash
{
namespace core
{

class ParticleSystemNodePrivate;

class CORESHARED_EXPORT ParticleSystemNode : public DrawableNode
{
    PIMPL(ParticleSystemNode)

public:
    class AbstractEmitter
    {
        NONCOPYBLE(AbstractEmitter)
    public:
        virtual ~AbstractEmitter() = default;
        virtual glm::vec3 operator ()() = 0;

        static std::shared_ptr<AbstractEmitter> buildPointEmitter();
        static std::shared_ptr<AbstractEmitter> buildRectEmitter(const glm::vec2&);
        static std::shared_ptr<AbstractEmitter> buildCircleEmitter(float);
        static std::shared_ptr<AbstractEmitter> buildBoxEmitter(const glm::vec3&);
        static std::shared_ptr<AbstractEmitter> buildSphereEmitter(float);

    protected:
        AbstractEmitter();
    };

    class AbstractUpdater
    {
        NONCOPYBLE(AbstractUpdater)
    public:
        virtual ~AbstractUpdater() = default;
        virtual uint32_t maxNumParticles() const = 0;
        virtual uint32_t numParticlesPerSecond() const = 0;

        virtual const glm::vec3& gravity() const = 0;

        static std::shared_ptr<AbstractUpdater> buildFireUpdater();

    protected:
        AbstractUpdater();
    };

    ParticleSystemNode(std::shared_ptr<AbstractEmitter>, std::shared_ptr<AbstractUpdater>);

};

} // namespace
} // namespace

#endif // PARTICLESYSTEMNODE_H
