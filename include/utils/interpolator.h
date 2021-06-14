#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <map>

#include <glm/common.hpp>

namespace trash
{
namespace utils
{

template <typename T>
class LinearInterpolator
{
public:
    LinearInterpolator() {}

    void addValue(float t, const T& v) {
        m_values.insert({t, v});
    }

    T interpolate(float t) const {
        auto upperBound = m_values.upper_bound(t);

        if (upperBound == m_values.cend())
            return (m_values.size()) ? (--m_values.cend())->second : T();

        if (upperBound == m_values.cbegin())
            return upperBound->second;

        auto leftNeighbor = upperBound;
        --leftNeighbor;

        const float coef = (t - leftNeighbor->first) / (upperBound->first - leftNeighbor->first);
        return glm::mix(leftNeighbor->second, upperBound->second, coef);
    }

private:
    std::map<float, T> m_values;
};


} // namespace
} // namespace

#endif // INTERPOLATOR_H
