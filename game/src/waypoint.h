#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <memory>

#include <glm/vec3.hpp>

namespace trash
{
namespace game
{

class WayPoint
{
public:
    WayPoint(const glm::vec3& = glm::vec3(), std::shared_ptr<WayPoint> = nullptr);

    void setPosition(const glm::vec3&);
    const glm::vec3& position() const;

    void setNextWayPoint(std::shared_ptr<WayPoint>);
    std::shared_ptr<WayPoint> nextWayPoint();
    std::shared_ptr<const WayPoint> nextWayPoint() const;

private:
    glm::vec3 m_position;
    std::weak_ptr<WayPoint> m_nextPoint;

};

} // namespace
} // namespace

#endif // WAYPOINT_H
