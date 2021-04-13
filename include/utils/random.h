#ifndef RANDOM_H
#define RANDOM_H

#include <random>

namespace trash
{
namespace utils
{

inline float random(const float from = 0.f, const float to = 1.f)
{
    static std::uniform_real_distribution<float> unifom(0.f, 1.f);
    static std::default_random_engine generator;

    return from + (to - from) * unifom(generator);
}

} // namespace
} // namespace

#endif // RANDOM_H
