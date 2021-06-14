#ifndef PARTICLESYSTEMNODE_H
#define PARTICLESYSTEMNODE_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <core/drawablenode.h>
#include <core/types.h>

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
        static std::shared_ptr<AbstractEmitter> buildQuadEmitter();
        static std::shared_ptr<AbstractEmitter> buildCircleEmitter();
        static std::shared_ptr<AbstractEmitter> buildBoxEmitter();
        static std::shared_ptr<AbstractEmitter> buildSphereEmitter();

    protected:
        AbstractEmitter();
    };

    void setDistanceAttenuation(bool, float);

    static std::shared_ptr<ParticleSystemNode> buildFire(std::shared_ptr<AbstractEmitter>);
    static std::shared_ptr<ParticleSystemNode> buildSmoke(std::shared_ptr<AbstractEmitter>);

protected:
    ParticleSystemNode(ParticleSystemNodePrivate*);

};

} // namespace
} // namespace

#endif // PARTICLESYSTEMNODE_H
