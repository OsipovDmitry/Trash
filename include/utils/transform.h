#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "../glm/mat4x4.hpp"
#include "../glm/vec3.hpp"
#include "../glm/gtc/quaternion.hpp"

namespace trash
{
namespace utils
{

struct Transform
{
public:
    glm::quat rotation;
    glm::vec3 scale;
    glm::vec3 translation;

    explicit Transform(const glm::vec3& s = glm::vec3(1.0f, 1.0f, 1.0f), const glm::quat& r = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), const glm::vec3& t = glm::vec3(0.0f, 0.0f, 0.0f))
        : rotation(r), scale(s), translation(t) {}

    glm::vec3 operator *(const glm::vec3& v) const {
        return translation + glm::vec3(rotation * glm::vec4((scale * v), 1.0f));
    }
    Transform operator *(const Transform& t2) const {
        return Transform(scale * t2.scale,
                         rotation * t2.rotation,
                         translation + scale * glm::vec3(rotation * glm::vec4(t2.translation, 1.0f)));
    }
    Transform& operator *=(const Transform& t2) {
        translation += scale * glm::vec3(rotation * glm::vec4(t2.translation, 1.0f));
        rotation *= t2.rotation;
        scale *= t2.scale;
        return *this;
    }

    operator glm::mat4x4() const {
        return
                glm::translate(glm::mat4x4(1.0f), translation) *
                glm::mat4_cast(rotation) *
                glm::scale(glm::mat4x4(1.0f), scale);
    }

    Transform& invert()
    {
        scale = 1.0f / scale;
        rotation = glm::conjugate(rotation);
        translation = -glm::vec3(rotation  * glm::vec4(translation * scale, 1.0f));
        return *this;
    }
    Transform inverted() const
    {
        return Transform(*this).invert();
    }

    static Transform fromScale(const glm::vec3& value) { return Transform(value, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)); }
    static Transform fromScale(float value) { return fromScale(glm::vec3(value, value, value)); }
    static Transform fromRotation(const glm::quat& value) { return Transform(glm::vec3(1.0f, 1.0f, 1.0f), value, glm::vec3(0.0f, 0.0f, 0.0f)); }
    static Transform fromTranslation(const glm::vec3& value) { return Transform(glm::vec3(1.0f, 1.0f, 1.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), value); }
};

inline glm::mat4x4 operator *(const glm::mat4x4& left, const Transform& right)
{
    return left * right.operator glm::mat4x4();
}

inline glm::mat4x4 operator *(const Transform& left, const glm::mat4x4& right)
{
    return left.operator glm::mat4x4() * right;
}

} // namespace
} // namespace

#endif // TRANSFORM_H
