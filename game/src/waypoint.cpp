#include "waypoint.h"

namespace trash
{
namespace game
{

WayPoint::WayPoint(const glm::vec3& pos, std::shared_ptr<WayPoint> next)
    : m_position(pos)
    , m_nextPoint(next)
{
}

void WayPoint::setPosition(const glm::vec3& value)
{
    m_position = value;
}

const glm::vec3& WayPoint::position() const
{
    return m_position;
}

void WayPoint::setNextWayPoint(std::shared_ptr<WayPoint> value)
{
    m_nextPoint = value;
}

std::shared_ptr<WayPoint> WayPoint::nextWayPoint()
{
    return m_nextPoint.lock();
}

std::shared_ptr<const WayPoint> WayPoint::nextWayPoint() const
{
    return m_nextPoint.lock();
}

} // namespace
} // namespace
